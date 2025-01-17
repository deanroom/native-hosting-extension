#ifdef _WIN32
#include <Windows.h>
#define PLATFORM_WINDOWS 1
#define MAX_PATH_LENGTH MAX_PATH
#else
#include <dlfcn.h>
#include <limits.h>
#define PLATFORM_UNIX 1
#define MAX_PATH_LENGTH PATH_MAX
#endif

#include "native_host.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <string>
#include <locale>
#include <codecvt>

#ifdef PLATFORM_WINDOWS
using char_t = wchar_t;
using LibraryHandle = HMODULE;
#else
using char_t = char;
using LibraryHandle = void *;
#endif

// Platform-specific utilities
namespace platform
{
    inline std::basic_string<char_t> to_char_t(const char *str)
    {
#ifdef PLATFORM_WINDOWS
        std::wstring wstr = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
        return std::basic_string<char_t>(wstr.begin(), wstr.end());
#else
        return std::basic_string<char_t>(str);
#endif
    }

    inline LibraryHandle load_library(const char_t *path)
    {
#ifdef PLATFORM_WINDOWS
        return LoadLibraryW(path);
#else
        std::string narrow_path(path);
        return dlopen(narrow_path.c_str(), RTLD_LAZY);
#endif
    }

    inline void *get_function(LibraryHandle lib, const char *name)
    {
#ifdef PLATFORM_WINDOWS
        return GetProcAddress(lib, name);
#else
        return dlsym(lib, name);
#endif
    }

    inline void free_library(LibraryHandle lib)
    {
#ifdef PLATFORM_WINDOWS
        FreeLibrary(lib);
#else
        dlclose(lib);
#endif
    }
}

// Runtime class to manage the .NET runtime initialization
class Runtime
{
public:
    static Runtime &instance()
    {
        static Runtime instance;
        return instance;
    }

    bool initialize()
    {
        if (is_initialized_)
            return true;

        // Get hostfxr path
        char_t hostfxr_path[MAX_PATH_LENGTH];
        size_t buffer_size = sizeof(hostfxr_path) / sizeof(char_t);

        int rc = get_hostfxr_path(hostfxr_path, &buffer_size, nullptr);
        if (rc != 0)
        {
            std::cerr << "Failed to get hostfxr path, error code: " << rc << std::endl;
            return false;
        }

        // Load hostfxr library
        LibraryHandle lib = platform::load_library(hostfxr_path);
        if (lib == nullptr)
        {
            std::cerr << "Failed to load hostfxr" << std::endl;
            return false;
        }

        // Get required functions
        auto init_fptr = (hostfxr_initialize_for_runtime_config_fn)
            platform::get_function(lib, "hostfxr_initialize_for_runtime_config");

        auto get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)
            platform::get_function(lib, "hostfxr_get_runtime_delegate");

        hostfxr_close_fn_ = (hostfxr_close_fn)
            platform::get_function(lib, "hostfxr_close");

        if (!init_fptr || !get_delegate_fptr || !hostfxr_close_fn_)
        {
            std::cerr << "Failed to get hostfxr function pointers" << std::endl;
            return false;
        }

        hostfxr_handle cxt = nullptr;
        auto config_path = platform::to_char_t(DEFAULT_CONFIG_PATH);
        rc = init_fptr(config_path.c_str(), nullptr, &cxt);

        if (rc != 0 && rc != 1)
        {
            hostfxr_close_fn_(cxt);
            std::cerr << "Failed to initialize runtime. Error code: " << rc << std::endl;
            return false;
        }

        rc = get_delegate_fptr(
            cxt,
            hdt_load_assembly_and_get_function_pointer,
            (void **)&load_assembly_fn_);

        if (rc != 0 || load_assembly_fn_ == nullptr)
        {
            hostfxr_close_fn_(cxt);
            std::cerr << "Failed to get load_assembly_and_get_function_ptr. Error code: " << rc << std::endl;
            return false;
        }

        is_initialized_ = true;
        hostfxr_close_fn_(cxt);
        return true;
    }

    load_assembly_and_get_function_pointer_fn get_load_assembly_fn() const
    {
        return load_assembly_fn_;
    }

    bool is_initialized() const { return is_initialized_; }

private:
    Runtime() : is_initialized_(false), load_assembly_fn_(nullptr), hostfxr_close_fn_(nullptr) {}
    ~Runtime() = default;
    Runtime(const Runtime &) = delete;
    Runtime &operator=(const Runtime &) = delete;

    bool is_initialized_;
    load_assembly_and_get_function_pointer_fn load_assembly_fn_;
    hostfxr_close_fn hostfxr_close_fn_;
    const char *const DEFAULT_CONFIG_PATH = "init.runtimeconfig.json";
};

// Assembly class representing a loaded .NET assembly
class Assembly
{
public:
    Assembly(const char *assembly_path) : assembly_path_(assembly_path) {}
    ~Assembly() = default;

    NativeHostStatus get_function_pointer(
        const char *type_name,
        const char *method_name,
        void **out_function_pointer)
    {
        if (!Runtime::instance().is_initialized())
        {
            std::cerr << "Runtime not initialized" << std::endl;
            return ERROR_RUNTIME_INIT;
        }

        auto load_assembly_fn = Runtime::instance().get_load_assembly_fn();
        *out_function_pointer = nullptr;

        auto assembly_path_t = platform::to_char_t(assembly_path_.c_str());
        auto type_name_t = platform::to_char_t(type_name);
        auto method_name_t = platform::to_char_t(method_name);
        std::cout << "type_name_t: " << type_name_t << std::endl;
        std::cout << "method_name_t: " << method_name_t << std::endl;

        int rc = load_assembly_fn(
            assembly_path_t.c_str(),
            type_name_t.c_str(),
            method_name_t.c_str(),
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr,
            out_function_pointer);

        if (rc != 0 || *out_function_pointer == nullptr)
        {
            std::cerr << "Failed to load assembly and get function pointer, errcode:" << rc << std::endl;

            // Map .NET error codes to our error codes
            constexpr int COR_E_FILENOTFOUND = -2146233079;
            constexpr int COR_E_TYPELOAD = -2146233054;
            constexpr int COR_E_MISSINGMETHOD = -2146233069;

            if (rc == COR_E_FILENOTFOUND)
                return ERROR_ASSEMBLY_LOAD;
            else if (rc == COR_E_TYPELOAD)
                return ERROR_TYPE_LOAD;
            else if (rc == COR_E_MISSINGMETHOD)
                return ERROR_METHOD_LOAD;

            return ERROR_METHOD_LOAD;
        }

        return SUCCESS;
    }

    const std::string &get_assembly_path() const { return assembly_path_; }

private:
    std::string assembly_path_;
};

// Host class managing assemblies
class Host
{
public:
    Host() = default;
    ~Host() = default;

    NativeHostStatus initialize_runtime()
    {
        if (!Runtime::instance().initialize())
        {
            return ERROR_RUNTIME_INIT;
        }
        return SUCCESS;
    }

    NativeHostStatus load_assembly(const char *assembly_path, native_assembly_handle_t *out_assembly_handle)
    {
        if (!assembly_path || !out_assembly_handle)
        {
            return ERROR_INVALID_ARG;
        }

        auto assembly = std::make_unique<Assembly>(assembly_path);
        *out_assembly_handle = reinterpret_cast<native_assembly_handle_t>(assembly.get());
        assemblies_[*out_assembly_handle] = std::move(assembly);
        return SUCCESS;
    }

    NativeHostStatus unload_assembly(native_assembly_handle_t assembly_handle)
    {
        if (!assembly_handle)
        {
            return ERROR_INVALID_ARG;
        }

        if (assemblies_.erase(assembly_handle) == 0)
        {
            return ERROR_assembly_NOT_FOUND;
        }

        return SUCCESS;
    }

    NativeHostStatus get_function_pointer(
        native_assembly_handle_t assembly_handle,
        const char *type_name,
        const char *method_name,
        void **out_function_pointer)
    {
        if (!assembly_handle || !type_name || !method_name || !out_function_pointer)
        {
            return ERROR_INVALID_ARG;
        }

        auto it = assemblies_.find(assembly_handle);
        if (it == assemblies_.end())
        {
            return ERROR_assembly_NOT_FOUND;
        }

        return it->second->get_function_pointer(
            type_name,
            method_name,
            out_function_pointer);
    }

private:
    std::unordered_map<native_assembly_handle_t, std::unique_ptr<Assembly>> assemblies_;
};

// Global instance management
static std::unique_ptr<Host> g_host;
static std::mutex g_mutex;

// Clean C exports
NATIVE_HOST_API NativeHostStatus create(native_host_handle_t *out_handle)
{
    if (!out_handle)
    {
        return ERROR_INVALID_ARG;
    }

    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_host)
    {
        return ERROR_HOST_ALREADY_EXISTS;
    }

    g_host = std::make_unique<Host>();
    *out_handle = reinterpret_cast<native_host_handle_t>(g_host.get());
    return SUCCESS;
}

NATIVE_HOST_API NativeHostStatus destroy(native_host_handle_t handle)
{
    if (!handle)
    {
        return ERROR_INVALID_ARG;
    }

    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_host || handle != reinterpret_cast<native_host_handle_t>(g_host.get()))
    {
        return ERROR_HOST_NOT_FOUND;
    }

    g_host.reset();
    return SUCCESS;
}

NATIVE_HOST_API NativeHostStatus initialize(
    native_host_handle_t host_handle)
{
    if (!host_handle)
    {
        return ERROR_INVALID_ARG;
    }

    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_host || host_handle != reinterpret_cast<native_host_handle_t>(g_host.get()))
    {
        return ERROR_HOST_NOT_FOUND;
    }

    return g_host->initialize_runtime();
}

NATIVE_HOST_API NativeHostStatus load(
    native_host_handle_t host_handle,
    const char *assembly_path,
    native_assembly_handle_t *out_assembly_handle)
{
    if (!host_handle)
    {
        return ERROR_INVALID_ARG;
    }

    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_host || host_handle != reinterpret_cast<native_host_handle_t>(g_host.get()))
    {
        return ERROR_HOST_NOT_FOUND;
    }

    return g_host->load_assembly(assembly_path, out_assembly_handle);
}

NATIVE_HOST_API NativeHostStatus unload(
    native_host_handle_t host_handle,
    native_assembly_handle_t assembly_handle)
{
    if (!host_handle)
    {
        return ERROR_INVALID_ARG;
    }

    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_host || host_handle != reinterpret_cast<native_host_handle_t>(g_host.get()))
    {
        return ERROR_HOST_NOT_FOUND;
    }

    return g_host->unload_assembly(assembly_handle);
}

NATIVE_HOST_API NativeHostStatus get_function_pointer(
    native_host_handle_t host_handle,
    native_assembly_handle_t assembly_handle,
    const char *type_name,
    const char *method_name,
    void **out_function_pointer)
{
    if (!host_handle)
    {
        return ERROR_INVALID_ARG;
    }

    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_host || host_handle != reinterpret_cast<native_host_handle_t>(g_host.get()))
    {
        return ERROR_HOST_NOT_FOUND;
    }

    return g_host->get_function_pointer(
        assembly_handle,
        type_name,
        method_name,
        out_function_pointer);
}