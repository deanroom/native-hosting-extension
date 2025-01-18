#ifdef _WIN32
#include <Windows.h>
#define MAX_PATH_LENGTH MAX_PATH
#else
#include <dlfcn.h>
#include <limits.h>
#define MAX_PATH_LENGTH PATH_MAX
#endif

#include "native_host.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <sstream>
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <filesystem>

namespace
{

#ifdef _WIN32
    using char_t = wchar_t;
    using lib_handle = HMODULE;
#else
    using char_t = char;
    using lib_handle = void *;
#endif

    void log_error(const std::string &message)
    {
        std::cerr << message << std::endl;
    }

    void log_error(const std::string &message, int error_code)
    {
        std::cerr << message << " (error code: " << error_code << ")" << std::endl;
    }

    void log_info(const std::string &message)
    {
#ifdef DEBUG
        std::cout << message << std::endl;
#endif
    }

    // Platform utilities with error handling
    lib_handle load_library(const char_t *path)
    {
#ifdef _WIN32
        lib_handle handle = LoadLibraryW(path);
        if (!handle)
        {
            log_error("LoadLibrary failed", GetLastError());
        }
        return handle;
#else
        lib_handle handle = dlopen(path, RTLD_LAZY);
        if (!handle)
        {
            log_error("dlopen failed", errno);
        }
        return handle;
#endif
    }

    void *get_function(lib_handle lib, const char *name)
    {
        void *fn = nullptr;
#ifdef _WIN32
        fn = GetProcAddress(lib, name);
        if (!fn)
        {
            log_error("GetProcAddress failed for " + std::string(name), GetLastError());
        }
#else
        fn = dlsym(lib, name);
        if (!fn)
        {
            log_error("dlsym failed for " + std::string(name));
        }
#endif
        return fn;
    }

    void free_library(lib_handle lib)
    {
#ifdef _WIN32
        if (!FreeLibrary(lib))
        {
            log_error("FreeLibrary failed", GetLastError());
        }
#else
        if (dlclose(lib) != 0)
        {
            log_error("dlclose failed", errno);
        }
#endif
    }

    std::basic_string<char_t> to_native_path(const char *path)
    {
#ifdef _WIN32
        int size = MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0);
        if (size == 0)
        {
            log_error("MultiByteToWideChar failed", GetLastError());
            return std::basic_string<char_t>();
        }
        std::wstring result(size - 1, 0);
        if (MultiByteToWideChar(CP_UTF8, 0, path, -1, &result[0], size) == 0)
        {
            log_error("MultiByteToWideChar failed", GetLastError());
            return std::basic_string<char_t>();
        }
        return result;
#else
        return path;
#endif
    }

    // .NET error code mapping
    namespace DotNetErrors
    {
        constexpr int FILE_NOT_FOUND = -2146233079;
        constexpr int TYPE_LOAD = -2146233054;
        constexpr int MISSING_METHOD = -2146233069;

        NativeHostStatus map_error(int error_code)
        {
            switch (error_code)
            {
            case FILE_NOT_FOUND:
                return NativeHostStatus::ERROR_ASSEMBLY_LOAD;
            case TYPE_LOAD:
                return NativeHostStatus::ERROR_TYPE_LOAD;
            case MISSING_METHOD:
                return NativeHostStatus::ERROR_METHOD_LOAD;
            default:
                return NativeHostStatus::ERROR_METHOD_LOAD;
            }
        }
    }

    // RAII wrapper for library handles
    class HostFxrLibrary
    {
        lib_handle handle_;

    public:
        explicit HostFxrLibrary(const char_t *path) : handle_(nullptr)
        {
            handle_ = load_library(path);
            if (handle_)
            {
                log_info("Loaded library: " + std::string(path));
            }
        }

        ~HostFxrLibrary()
        {
            if (handle_)
            {
                free_library(handle_);
                log_info("Unloaded library");
            }
        }

        lib_handle get() const { return handle_; }
        operator bool() const { return handle_ != nullptr; }
    };

    // Enhanced runtime management
    class Runtime
    {
        bool initialized_ = false;
        load_assembly_and_get_function_pointer_fn load_assembly_fn_ = nullptr;
        hostfxr_close_fn close_fn_ = nullptr;
        std::unique_ptr<HostFxrLibrary> hostfxr_lib_;
        static constexpr const char *config_path = "init.runtimeconfig.json";

        bool load_hostfxr()
        {
            char_t hostfxr_path[MAX_PATH_LENGTH];
            size_t buffer_size = sizeof(hostfxr_path) / sizeof(char_t);

            int rc = get_hostfxr_path(hostfxr_path, &buffer_size, nullptr);
            if (rc != 0)
            {
                log_error("Failed to get hostfxr path", rc);
                return false;
            }

            hostfxr_lib_ = std::make_unique<HostFxrLibrary>(hostfxr_path);
            if (!hostfxr_lib_ || !*hostfxr_lib_)
            {
                log_error("Failed to load hostfxr library");
                return false;
            }

            auto init_fn = (hostfxr_initialize_for_runtime_config_fn)
                get_function(hostfxr_lib_->get(), "hostfxr_initialize_for_runtime_config");
            auto get_delegate_fn = (hostfxr_get_runtime_delegate_fn)
                get_function(hostfxr_lib_->get(), "hostfxr_get_runtime_delegate");
            close_fn_ = (hostfxr_close_fn)
                get_function(hostfxr_lib_->get(), "hostfxr_close");

            if (!init_fn || !get_delegate_fn || !close_fn_)
            {
                log_error("Failed to get required functions");
                return false;
            }

            hostfxr_handle ctx = nullptr;
            rc = init_fn(to_native_path(config_path).c_str(), nullptr, &ctx);

            // rc返回值为1时，也是 hostfxr已经初始化，只是使用了相同配置，
            // 但是当前实现只有一个 runtime 单例，所以这里不会出现此种情形。
            if (rc != 0)
            {
                log_error("Failed to initialize runtime", rc);
                return false;
            }

            rc = get_delegate_fn(
                ctx,
                hdt_load_assembly_and_get_function_pointer,
                (void **)&load_assembly_fn_);

            if (rc != 0 || !load_assembly_fn_)
            {
                close_fn_(ctx);
                log_error("Failed to get load assembly function", rc);
                return false;
            }

            close_fn_(ctx);
            log_info("Runtime initialized successfully");
            return true;
        }

    public:
        static Runtime &instance()
        {
            static Runtime runtime;
            return runtime;
        }

        bool initialize()
        {
            if (initialized_)
                return true;
            if (!load_hostfxr())
                return false;
            initialized_ = true;
            return true;
        }

        load_assembly_and_get_function_pointer_fn get_load_fn() const { return load_assembly_fn_; }
        bool is_initialized() const { return initialized_; }
    };

    // Enhanced assembly management
    class Assembly
    {
        std::string path_;
        bool loaded_ = false;

    public:
        explicit Assembly(const char *path) : path_(path)
        {
            log_info("Created assembly for path: " + path_);
        }

        ~Assembly()
        {
            log_info("Destroying assembly: " + path_);
        }

        NativeHostStatus get_function(const char *type_name, const char *method_name, void **fn_ptr)
        {
            if (!Runtime::instance().is_initialized())
            {
                log_error("Runtime not initialized");
                return NativeHostStatus::ERROR_RUNTIME_INIT;
            }

            auto load_fn = Runtime::instance().get_load_fn();
            *fn_ptr = nullptr;

            // Check if assembly file exists
            if (!std::filesystem::exists(path_))
            {
                log_error("Assembly file not found: " + path_);
                return NativeHostStatus::ERROR_ASSEMBLY_LOAD;
            }

            log_info("Loading type: " + std::string(type_name));
            log_info("Loading method: " + std::string(method_name));

            int rc = load_fn(
                to_native_path(path_.c_str()).c_str(),
                to_native_path(type_name).c_str(),
                to_native_path(method_name).c_str(),
                UNMANAGEDCALLERSONLY_METHOD,
                nullptr,
                fn_ptr);

            if (rc != 0 || !*fn_ptr)
            {
                log_error("Failed to load assembly and get function pointer", rc);
                return DotNetErrors::map_error(rc);
            }

            loaded_ = true;
            log_info("Successfully loaded function");
            return NativeHostStatus::SUCCESS;
        }

        bool is_loaded() const { return loaded_; }
        const std::string &path() const { return path_; }
    };

    // Enhanced host management
    class Host
    {
        std::unordered_map<native_assembly_handle_t, std::unique_ptr<Assembly>> assemblies_;
        bool initialized_ = false;

    public:
        NativeHostStatus initialize_runtime()
        {
            if (initialized_)
            {
                log_info("Runtime already initialized");
                return NativeHostStatus::SUCCESS;
            }

            if (!Runtime::instance().initialize())
            {
                log_error("Failed to initialize runtime");
                return NativeHostStatus::ERROR_RUNTIME_INIT;
            }

            initialized_ = true;
            log_info("Host runtime initialized successfully");
            return NativeHostStatus::SUCCESS;
        }

        NativeHostStatus load_assembly(const char *path, native_assembly_handle_t *handle)
        {
            if (!path || !handle)
            {
                log_error("Invalid arguments for load_assembly");
                return NativeHostStatus::ERROR_INVALID_ARG;
            }

            if (!initialized_)
            {
                log_error("Runtime not initialized");
                return NativeHostStatus::ERROR_ASSEMBLY_NOT_INITIALIZED;
            }

            // Check if assembly file exists
            if (!std::filesystem::exists(path))
            {
                log_error("Assembly file not found: " + std::string(path));
                return NativeHostStatus::ERROR_ASSEMBLY_LOAD;
            }

            auto assembly = std::make_unique<Assembly>(path);
            *handle = assembly.get();
            assemblies_[*handle] = std::move(assembly);
            log_info("Assembly loaded successfully: " + std::string(path));
            return NativeHostStatus::SUCCESS;
        }

        NativeHostStatus unload_assembly(native_assembly_handle_t handle)
        {
            if (!handle)
            {
                log_error("Invalid handle for unload_assembly");
                return NativeHostStatus::ERROR_INVALID_ARG;
            }

            auto count = assemblies_.erase(handle);
            if (count == 0)
            {
                log_error("Assembly not found for unload");
                return NativeHostStatus::ERROR_ASSEMBLY_NOT_FOUND;
            }

            log_info("Assembly unloaded successfully");
            return NativeHostStatus::SUCCESS;
        }

        NativeHostStatus get_function_pointer(
            native_assembly_handle_t handle,
            const char *type_name,
            const char *method_name,
            void **fn_ptr)
        {
            if (!handle || !type_name || !method_name || !fn_ptr)
            {
                log_error("Invalid arguments for get_function_pointer");
                return NativeHostStatus::ERROR_INVALID_ARG;
            }

            auto it = assemblies_.find(handle);
            if (it == assemblies_.end())
            {
                log_error("Assembly not found for get_function_pointer");
                return NativeHostStatus::ERROR_ASSEMBLY_NOT_FOUND;
            }

            return it->second->get_function(type_name, method_name, fn_ptr);
        }

        size_t assembly_count() const { return assemblies_.size(); }
        bool is_initialized() const { return initialized_; }
    };

    // Global state with enhanced management
    std::unique_ptr<Host> g_host;
    std::mutex g_mutex;
}

// Enhanced API implementations
extern "C"
{
    NATIVE_HOST_API NativeHostStatus create(native_host_handle_t *out_handle)
    {
        if (!out_handle)
        {
            log_error("Invalid out_handle for create");
            return NativeHostStatus::ERROR_INVALID_ARG;
        }

        std::lock_guard<std::mutex> lock(g_mutex);
        if (g_host)
        {
            log_error("Host already exists");
            return NativeHostStatus::ERROR_HOST_ALREADY_EXISTS;
        }

        g_host = std::make_unique<Host>();
        *out_handle = g_host.get();
        log_info("Host created successfully");
        return NativeHostStatus::SUCCESS;
    }

    NATIVE_HOST_API NativeHostStatus destroy(native_host_handle_t handle)
    {
        if (!handle)
        {
            log_error("Invalid handle for destroy");
            return NativeHostStatus::ERROR_INVALID_ARG;
        }

        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_host || handle != g_host.get())
        {
            log_error("Host not found for destroy");
            return NativeHostStatus::ERROR_HOST_NOT_FOUND;
        }

        g_host.reset();
        log_info("Host destroyed successfully");
        return NativeHostStatus::SUCCESS;
    }

    NATIVE_HOST_API NativeHostStatus initialize(native_host_handle_t handle)
    {
        if (!handle)
        {
            log_error("Invalid handle for initialize");
            return NativeHostStatus::ERROR_INVALID_ARG;
        }

        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_host || handle != g_host.get())
        {
            log_error("Host not found for initialize");
            return NativeHostStatus::ERROR_HOST_NOT_FOUND;
        }

        return g_host->initialize_runtime();
    }

    NATIVE_HOST_API NativeHostStatus load(
        native_host_handle_t handle,
        const char *path,
        native_assembly_handle_t *out_handle)
    {
        if (!handle)
        {
            log_error("Invalid handle for load");
            return NativeHostStatus::ERROR_INVALID_ARG;
        }

        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_host || handle != g_host.get())
        {
            log_error("Host not found for load");
            return NativeHostStatus::ERROR_HOST_NOT_FOUND;
        }

        return g_host->load_assembly(path, out_handle);
    }

    NATIVE_HOST_API NativeHostStatus unload(
        native_host_handle_t handle,
        native_assembly_handle_t assembly)
    {
        if (!handle)
        {
            log_error("Invalid handle for unload");
            return NativeHostStatus::ERROR_INVALID_ARG;
        }

        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_host || handle != g_host.get())
        {
            log_error("Host not found for unload");
            return NativeHostStatus::ERROR_HOST_NOT_FOUND;
        }

        return g_host->unload_assembly(assembly);
    }

    NATIVE_HOST_API NativeHostStatus get_function_pointer(
        native_host_handle_t handle,
        native_assembly_handle_t assembly,
        const char *type_name,
        const char *method_name,
        void **fn_ptr)
    {
        if (!handle)
        {
            log_error("Invalid handle for get_function_pointer");
            return NativeHostStatus::ERROR_INVALID_ARG;
        }

        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_host || handle != g_host.get())
        {
            log_error("Host not found for get_function_pointer");
            return NativeHostStatus::ERROR_HOST_NOT_FOUND;
        }

        return g_host->get_function_pointer(assembly, type_name, method_name, fn_ptr);
    }
}