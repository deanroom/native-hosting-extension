#include "native_plugin_host.h"
#include "platform.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

using namespace native_plugin_host;

// Plugin instance implementation
class NativePlugin
{
public:
    NativePlugin() : is_initialized(false), load_assembly_and_get_function_ptr(nullptr), hostfxr_close_ptr(nullptr) {}

    bool isInitialized() const { return is_initialized; }

    void setInitialized(bool value) { is_initialized = value; }

    void setLoadAssemblyFunction(load_assembly_and_get_function_pointer_fn fn)
    {
        load_assembly_and_get_function_ptr = fn;
    }

    void setHostfxrCloseFunction(hostfxr_close_fn fn)
    {
        hostfxr_close_ptr = fn;
    }

    load_assembly_and_get_function_pointer_fn getLoadAssemblyFunction() const
    {
        return load_assembly_and_get_function_ptr;
    }

private:
    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_ptr;
    hostfxr_close_fn hostfxr_close_ptr;
    bool is_initialized;
};

// Host instance implementation
class NativeHost
{
public:
    NativeHost() {}

    ~NativeHost()
    {
        plugins.clear();
    }

    NativePlugin *getPlugin(native_plugin_handle_t handle)
    {
        auto it = plugins.find(handle);
        return it != plugins.end() ? it->second.get() : nullptr;
    }

    native_plugin_handle_t addPlugin(std::unique_ptr<NativePlugin> plugin)
    {
        native_plugin_handle_t handle = plugin.get();
        plugins[handle] = std::move(plugin);
        return handle;
    }

    bool removePlugin(native_plugin_handle_t handle)
    {
        return plugins.erase(handle) > 0;
    }

private:
    std::unordered_map<native_plugin_handle_t, std::unique_ptr<NativePlugin>> plugins;
};

// Global instance management
static std::unordered_map<native_host_handle_t, std::unique_ptr<NativeHost>> g_hosts;
static std::mutex g_hosts_mutex;

// Implementation of the interface
extern "C"
{

    NativePluginHostStatus NATIVE_PLUGIN_HOST_API native_plugin_host_create(native_host_handle_t *out_handle)
    {
        if (!out_handle)
        {
            return NATIVE_PLUGIN_HOST_ERROR_INVALID_ARG;
        }

        auto host = std::make_unique<NativeHost>();
        native_host_handle_t handle = host.get();

        std::lock_guard<std::mutex> lock(g_hosts_mutex);
        g_hosts[handle] = std::move(host);
        *out_handle = handle;

        return NATIVE_PLUGIN_HOST_SUCCESS;
    }

    NativePluginHostStatus NATIVE_PLUGIN_HOST_API native_plugin_host_destroy(native_host_handle_t handle)
    {
        if (!handle)
        {
            return NATIVE_PLUGIN_HOST_ERROR_INVALID_ARG;
        }

        std::lock_guard<std::mutex> lock(g_hosts_mutex);
        auto it = g_hosts.find(handle);
        if (it == g_hosts.end())
        {
            return NATIVE_PLUGIN_HOST_ERROR_HOST_NOT_FOUND;
        }

        g_hosts.erase(it);
        return NATIVE_PLUGIN_HOST_SUCCESS;
    }

    NativePluginHostStatus NATIVE_PLUGIN_HOST_API native_plugin_host_load(
        native_host_handle_t host_handle,
        const char *runtime_config_path,
        native_plugin_handle_t *out_plugin_handle)
    {

        if (!host_handle || !runtime_config_path || !out_plugin_handle)
        {
            return NATIVE_PLUGIN_HOST_ERROR_INVALID_ARG;
        }

        std::lock_guard<std::mutex> lock(g_hosts_mutex);
        auto host_it = g_hosts.find(host_handle);
        if (host_it == g_hosts.end())
        {
            return NATIVE_PLUGIN_HOST_ERROR_HOST_NOT_FOUND;
        }

        auto plugin = std::make_unique<NativePlugin>();

        // Get hostfxr path
        char_t hostfxr_path[MAX_PATH_LENGTH];
        size_t buffer_size = sizeof(hostfxr_path) / sizeof(char_t);

        int rc = get_hostfxr_path(hostfxr_path, &buffer_size, nullptr);
        if (rc != 0)
        {
            std::cerr << "Failed to get hostfxr path, error code: " << rc << std::endl;
            return NATIVE_PLUGIN_HOST_ERROR_HOSTFXR_NOT_FOUND;
        }

        // Load hostfxr library
        LibraryHandle lib = native_plugin_host::load_library(hostfxr_path);
        if (lib == nullptr)
        {
            std::cerr << "Failed to load hostfxr" << std::endl;
            return NATIVE_PLUGIN_HOST_ERROR_HOSTFXR_NOT_FOUND;
        }

        // Get required functions
        auto init_fptr = (hostfxr_initialize_for_runtime_config_fn)
            native_plugin_host::get_function(lib, "hostfxr_initialize_for_runtime_config");

        auto get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)
            native_plugin_host::get_function(lib, "hostfxr_get_runtime_delegate");

        auto hostfxr_close_ptr = (hostfxr_close_fn)
            native_plugin_host::get_function(lib, "hostfxr_close");

        if (!init_fptr || !get_delegate_fptr || !hostfxr_close_ptr)
        {
            std::cerr << "Failed to get hostfxr function pointers" << std::endl;
            return NATIVE_PLUGIN_HOST_ERROR_DELEGATE_NOT_FOUND;
        }

        plugin->setHostfxrCloseFunction(hostfxr_close_ptr);

        hostfxr_handle cxt = nullptr;
        auto config_path = to_char_t(runtime_config_path);
        rc = init_fptr(config_path.c_str(), nullptr, &cxt);

        if (rc != 0 && rc != 1)
        {
            hostfxr_close_ptr(cxt);
            std::cerr << "Failed to initialize runtime. Error code: " << rc << std::endl;
            return NATIVE_PLUGIN_HOST_ERROR_RUNTIME_INIT;
        }

        std::cout << "Runtime initialized successfully, with ctx id:" << cxt << std::endl;

        load_assembly_and_get_function_pointer_fn load_assembly_fn = nullptr;
        rc = get_delegate_fptr(
            cxt,
            hdt_load_assembly_and_get_function_pointer,
            (void **)&load_assembly_fn);

        if (rc != 0 || load_assembly_fn == nullptr)
        {
            hostfxr_close_ptr(cxt);
            std::cerr << "Failed to get load_assembly_and_get_function_ptr. Error code: " << rc << std::endl;
            return NATIVE_PLUGIN_HOST_ERROR_DELEGATE_NOT_FOUND;
        }

        plugin->setLoadAssemblyFunction(load_assembly_fn);
        plugin->setInitialized(true);
        hostfxr_close_ptr(cxt);

        *out_plugin_handle = host_it->second->addPlugin(std::move(plugin));
        return NATIVE_PLUGIN_HOST_SUCCESS;
    }

    NativePluginHostStatus NATIVE_PLUGIN_HOST_API native_plugin_host_unload(
        native_host_handle_t host_handle,
        native_plugin_handle_t plugin_handle)
    {

        if (!host_handle || !plugin_handle)
        {
            return NATIVE_PLUGIN_HOST_ERROR_INVALID_ARG;
        }

        std::lock_guard<std::mutex> lock(g_hosts_mutex);
        auto host_it = g_hosts.find(host_handle);
        if (host_it == g_hosts.end())
        {
            return NATIVE_PLUGIN_HOST_ERROR_HOST_NOT_FOUND;
        }

        if (!host_it->second->removePlugin(plugin_handle))
        {
            return NATIVE_PLUGIN_HOST_ERROR_PLUGIN_NOT_FOUND;
        }

        return NATIVE_PLUGIN_HOST_SUCCESS;
    }

    NativePluginHostStatus NATIVE_PLUGIN_HOST_API native_plugin_host_get_function_pointer(
        native_host_handle_t host_handle,
        native_plugin_handle_t plugin_handle,
        const char *assembly_path,
        const char *type_name,
        const char *method_name,
        const char *delegate_type_name,
        void **out_function_pointer)
    {

        if (!host_handle || !plugin_handle || !assembly_path || !type_name ||
            !method_name || !delegate_type_name || !out_function_pointer)
        {
            return NATIVE_PLUGIN_HOST_ERROR_INVALID_ARG;
        }

        std::lock_guard<std::mutex> lock(g_hosts_mutex);
        auto host_it = g_hosts.find(host_handle);
        if (host_it == g_hosts.end())
        {
            return NATIVE_PLUGIN_HOST_ERROR_HOST_NOT_FOUND;
        }

        auto plugin = host_it->second->getPlugin(plugin_handle);
        if (!plugin)
        {
            return NATIVE_PLUGIN_HOST_ERROR_PLUGIN_NOT_FOUND;
        }

        if (!plugin->isInitialized())
        {
            std::cerr << "Plugin not initialized" << std::endl;
            return NATIVE_PLUGIN_HOST_ERROR_PLUGIN_NOT_INITIALIZED;
        }

        auto load_assembly_fn = plugin->getLoadAssemblyFunction();
        *out_function_pointer = nullptr;

        auto assembly_path_t = to_char_t(assembly_path);
        auto type_name_t = to_char_t(type_name);
        auto method_name_t = to_char_t(method_name);
        auto delegate_type_name_t = to_char_t(delegate_type_name);

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
                return NATIVE_PLUGIN_HOST_ERROR_ASSEMBLY_LOAD;
            else if (rc == COR_E_TYPELOAD)
                return NATIVE_PLUGIN_HOST_ERROR_TYPE_LOAD;
            else if (rc == COR_E_MISSINGMETHOD)
                return NATIVE_PLUGIN_HOST_ERROR_METHOD_LOAD;

            return NATIVE_PLUGIN_HOST_ERROR_METHOD_LOAD;
        }

        return NATIVE_PLUGIN_HOST_SUCCESS;
    }
}