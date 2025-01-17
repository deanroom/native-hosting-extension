#pragma once

/**
 * @file native_host.h
 * @brief .NET运行时的原生插件宿主接口
 *
 * 此头文件提供了从原生代码托管和交互.NET Native AOT编译程序集的核心功能。
 * 它支持加载.NET程序集、初始化运行时以及获取托管方法的函数指针。
 */

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

#include <string>

/**
 * @brief 原生托管API的错误代码
 */
typedef enum NativeHostStatus
{
    // 成功代码（0和正值）
    SUCCESS = 0, ///< 操作成功完成

    // 宿主错误（-100到-199）
    ERROR_HOST_NOT_FOUND = -100,      ///< 未找到宿主实例
    ERROR_HOST_ALREADY_EXISTS = -101, ///< 宿主实例已存在

    // 插件错误（-200到-299）
    ERROR_assembly_NOT_FOUND = -200,       ///< 未找到插件
    ERROR_assembly_NOT_INITIALIZED = -203, ///< 插件未初始化

    // 运行时错误（-300到-399）
    ERROR_RUNTIME_INIT = -300,       ///< 运行时初始化失败
    ERROR_HOSTFXR_NOT_FOUND = -302,  ///< 未找到hostfxr库
    ERROR_DELEGATE_NOT_FOUND = -303, ///< 获取所需函数委托失败

    // 程序集错误（-400到-499）
    ERROR_ASSEMBLY_LOAD = -400, ///< 加载程序集失败
    ERROR_TYPE_LOAD = -401,     ///< 加载类型失败
    ERROR_METHOD_LOAD = -402,   ///< 加载方法失败

    // 参数错误（-500到-599）
    ERROR_INVALID_ARG = -500 ///< 无效参数
} NativeHostStatus;

/**
 * @brief 原生宿主实例的不透明句柄类型
 */
typedef void *native_host_handle_t;

/**
 * @brief 插件实例的不透明句柄类型
 */
typedef void *native_assembly_handle_t;

extern "C"
{
    /**
     * @brief Create a new native host instance
     * @param[out] out_handle Pointer to store the resulting host handle
     * @return NativeHostStatus indicating success or specific error
     */
    NATIVE_HOST_API NativeHostStatus create(native_host_handle_t *out_handle);

    /**
     * @brief Destroy a native host instance and clean up all its assemblys
     * @param handle The host instance handle to destroy
     * @return NativeHostStatus indicating success or specific error
     */
    NATIVE_HOST_API NativeHostStatus destroy(native_host_handle_t handle);

    /**
     * @brief Initialize the .NET runtime with the specified configuration
     * @param host_handle The host instance handle
     * @return NativeHostStatus indicating success or specific error
     */
    NATIVE_HOST_API NativeHostStatus initialize(
        native_host_handle_t host_handle);

    /**
     * @brief Load a .NET assembly into the host
     * @param host_handle The host instance handle
     * @param assembly_path Path to the .NET assembly file
     * @param[out] out_assembly_handle Pointer to store the resulting assembly handle
     * @return NativeHostStatus indicating success or specific error
     */
    NATIVE_HOST_API NativeHostStatus load(
        native_host_handle_t host_handle,
        const char *assembly_path,
        native_assembly_handle_t *out_assembly_handle);

    /**
     * @brief Unload a assembly from the host and clean up its resources
     * @param host_handle The host instance handle
     * @param assembly_handle The assembly handle to unload
     * @return NativeHostStatus indicating success or specific error
     */
    NATIVE_HOST_API NativeHostStatus unload(
        native_host_handle_t host_handle,
        native_assembly_handle_t assembly_handle);

    /**
     * @brief Get a function pointer to a specific method from a loaded assembly
     * @param host_handle The host instance handle
     * @param assembly_handle The assembly handle
     * @param type_name Fully qualified name of the type containing the method
     * @param method_name Name of the method to get a pointer to
     * @param[out] out_function_pointer Pointer to store the resulting function pointer
     * @return NativeHostStatus indicating success or specific error
     */
    NATIVE_HOST_API NativeHostStatus get_function_pointer(
        native_host_handle_t host_handle,
        native_assembly_handle_t assembly_handle,
        const char *type_name,
        const char *method_name,
        void **out_function_pointer);
}