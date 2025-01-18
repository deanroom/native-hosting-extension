#pragma once

#ifdef _WIN32
#ifdef NATIVE_HOST_EXPORTS
#define NATIVE_HOST_API __declspec(dllexport)
#else
#define NATIVE_HOST_API __declspec(dllimport)
#endif
#else
#ifdef NATIVE_HOST_EXPORTS
#define NATIVE_HOST_API __attribute__((visibility("default")))
#else
#define NATIVE_HOST_API
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Native host API status codes
 */
enum NativeHostStatus
{
    SUCCESS = 0,
    ERROR_HOST_NOT_FOUND = -100,
    ERROR_HOST_ALREADY_EXISTS = -101,
    ERROR_ASSEMBLY_NOT_FOUND = -200,
    ERROR_ASSEMBLY_NOT_INITIALIZED = -203,
    ERROR_RUNTIME_INIT = -300,
    ERROR_HOSTFXR_NOT_FOUND = -302,
    ERROR_DELEGATE_NOT_FOUND = -303,
    ERROR_ASSEMBLY_LOAD = -400,
    ERROR_TYPE_LOAD = -401,
    ERROR_METHOD_LOAD = -402,
    ERROR_INVALID_ARG = -500
};

typedef void* native_handle_t;
typedef native_handle_t native_host_handle_t;
typedef native_handle_t native_assembly_handle_t;

NATIVE_HOST_API enum NativeHostStatus create(native_host_handle_t* out_handle);
NATIVE_HOST_API enum NativeHostStatus destroy(native_host_handle_t handle);
NATIVE_HOST_API enum NativeHostStatus initialize(native_host_handle_t handle);
NATIVE_HOST_API enum NativeHostStatus load(native_host_handle_t handle, const char* assembly_path, native_assembly_handle_t* out_assembly_handle);
NATIVE_HOST_API enum NativeHostStatus unload(native_host_handle_t handle, native_assembly_handle_t assembly_handle);
NATIVE_HOST_API enum NativeHostStatus get_function_pointer(native_host_handle_t handle, native_assembly_handle_t assembly_handle, const char* type_name, const char* method_name, void** out_function_pointer);

#ifdef __cplusplus
}
#endif