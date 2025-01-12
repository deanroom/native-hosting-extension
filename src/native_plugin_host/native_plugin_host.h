#pragma once

/**
 * @file native_plugin_host.h
 * @brief Native Plugin Host Interface for .NET Runtime
 *
 * This header provides the core functionality for hosting and interacting with
 * .NET Native AOT compiled assemblies from native code. It enables loading
 * .NET assemblies, initializing the runtime, and obtaining function pointers
 * to managed methods.
 */

#ifdef _WIN32
#ifdef NATIVE_PLUGIN_HOST_EXPORTS
#define NATIVE_PLUGIN_HOST_API __declspec(dllexport)
#else
#define NATIVE_PLUGIN_HOST_API __declspec(dllimport)
#endif
#else
#ifdef NATIVE_PLUGIN_HOST_EXPORTS
#define NATIVE_PLUGIN_HOST_API __attribute__((visibility("default")))
#else
#define NATIVE_PLUGIN_HOST_API
#endif
#endif

#include <string>

/**
 * @brief Error codes for the native hosting API
 */
typedef enum
{
    // Success codes (0 and positive values)
    NATIVE_PLUGIN_HOST_SUCCESS = 0, ///< Operation completed successfully

    // Host errors (-100 to -199)
    NATIVE_PLUGIN_HOST_ERROR_HOST_NOT_FOUND = -100, ///< Host instance not found

    // Plugin errors (-200 to -299)
    NATIVE_PLUGIN_HOST_ERROR_PLUGIN_NOT_FOUND = -200,       ///< Plugin not found
    NATIVE_PLUGIN_HOST_ERROR_PLUGIN_NOT_INITIALIZED = -203, ///< Plugin is not initialized

    // Runtime errors (-300 to -399)
    NATIVE_PLUGIN_HOST_ERROR_RUNTIME_INIT = -300,       ///< Failed to initialize runtime
    NATIVE_PLUGIN_HOST_ERROR_HOSTFXR_NOT_FOUND = -302,  ///< hostfxr library not found
    NATIVE_PLUGIN_HOST_ERROR_DELEGATE_NOT_FOUND = -303, ///< Failed to get required function delegate

    // Assembly errors (-400 to -499)
    NATIVE_PLUGIN_HOST_ERROR_ASSEMBLY_LOAD = -400, ///< Failed to load assembly
    NATIVE_PLUGIN_HOST_ERROR_TYPE_LOAD = -401,     ///< Failed to load type
    NATIVE_PLUGIN_HOST_ERROR_METHOD_LOAD = -402,   ///< Failed to load method

    // General errors (-500 to -599)
    NATIVE_PLUGIN_HOST_ERROR_INVALID_ARG = -500, ///< Invalid argument provided
} NativePluginHostStatus;

/**
 * @brief Opaque handle type for native host instance
 */
typedef void *native_host_handle_t;

/**
 * @brief Opaque handle type for plugin instance
 */
typedef void *native_plugin_handle_t;

extern "C"
{
    /**
     * @brief Create a new native host instance
     * @param[out] out_handle Pointer to store the resulting host handle
     * @return NativePluginHostStatus indicating success or specific error
     */
    NATIVE_PLUGIN_HOST_API NativePluginHostStatus native_plugin_host_create(native_host_handle_t *out_handle);

    /**
     * @brief Destroy a native host instance and cleanup all its plugins
     * @param handle The host instance handle to destroy
     * @return NativePluginHostStatus indicating success or specific error
     */
    NATIVE_PLUGIN_HOST_API NativePluginHostStatus native_plugin_host_destroy(native_host_handle_t handle);

    /**
     * @brief Load a plugin into the host with the specified runtime configuration
     * @param host_handle The host instance handle
     * @param runtime_config_path Path to the runtime configuration JSON file
     * @param[out] out_plugin_handle Pointer to store the resulting plugin handle
     * @return NativePluginHostStatus indicating success or specific error
     */
    NATIVE_PLUGIN_HOST_API NativePluginHostStatus native_plugin_host_load(
        native_host_handle_t host_handle,
        const char *runtime_config_path,
        native_plugin_handle_t *out_plugin_handle);

    /**
     * @brief Unload a plugin from the host and cleanup its resources
     * @param host_handle The host instance handle
     * @param plugin_handle The plugin handle to unload
     * @return NativePluginHostStatus indicating success or specific error
     */
    NATIVE_PLUGIN_HOST_API NativePluginHostStatus native_plugin_host_unload(
        native_host_handle_t host_handle,
        native_plugin_handle_t plugin_handle);

    /**
     * @brief Load a .NET assembly and get a function pointer to a specific method from a plugin
     * @param host_handle The host instance handle
     * @param plugin_handle The plugin handle
     * @param assembly_path Path to the .NET assembly file
     * @param type_name Fully qualified name of the type containing the method
     * @param method_name Name of the method to get a pointer to
     * @param delegate_type_name Fully qualified name of the delegate type matching the method signature
     * @param[out] out_function_pointer Pointer to store the resulting function pointer
     * @return NativePluginHostStatus indicating success or specific error
     */
    NATIVE_PLUGIN_HOST_API NativePluginHostStatus native_plugin_host_get_function_pointer(
        native_host_handle_t host_handle,
        native_plugin_handle_t plugin_handle,
        const char *assembly_path,
        const char *type_name,
        const char *method_name,
        const char *delegate_type_name,
        void **out_function_pointer);
}