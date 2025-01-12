#pragma once

/**
 * @file native_plugin_host.h
 * @brief .NET运行时的原生插件宿主接口
 *
 * 此头文件提供了从原生代码托管和交互.NET Native AOT编译程序集的核心功能。
 * 它支持加载.NET程序集、初始化运行时以及获取托管方法的函数指针。
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
 * @brief 原生托管API的错误代码
 */
typedef enum
{
    // 成功代码（0和正值）
    NATIVE_PLUGIN_HOST_SUCCESS = 0, ///< 操作成功完成

    // 宿主错误（-100到-199）
    NATIVE_PLUGIN_HOST_ERROR_HOST_NOT_FOUND = -100, ///< 未找到宿主实例

    // 插件错误（-200到-299）
    NATIVE_PLUGIN_HOST_ERROR_PLUGIN_NOT_FOUND = -200,       ///< 未找到插件
    NATIVE_PLUGIN_HOST_ERROR_PLUGIN_NOT_INITIALIZED = -203, ///< 插件未初始化

    // 运行时错误（-300到-399）
    NATIVE_PLUGIN_HOST_ERROR_RUNTIME_INIT = -300,       ///< 运行时初始化失败
    NATIVE_PLUGIN_HOST_ERROR_HOSTFXR_NOT_FOUND = -302,  ///< 未找到hostfxr库
    NATIVE_PLUGIN_HOST_ERROR_DELEGATE_NOT_FOUND = -303, ///< 获取所需函数委托失败

    // 程序集错误（-400到-499）
    NATIVE_PLUGIN_HOST_ERROR_ASSEMBLY_LOAD = -400, ///< 加载程序集失败
    NATIVE_PLUGIN_HOST_ERROR_TYPE_LOAD = -401,     ///< 加载类型失败
    NATIVE_PLUGIN_HOST_ERROR_METHOD_LOAD = -402,   ///< 加载方法失败

    // 通用错误（-500到-599）
    NATIVE_PLUGIN_HOST_ERROR_INVALID_ARG = -500, ///< 提供了无效参数
} NativePluginHostStatus;

/**
 * @brief 原生宿主实例的不透明句柄类型
 */
typedef void *native_host_handle_t;

/**
 * @brief 插件实例的不透明句柄类型
 */
typedef void *native_plugin_handle_t;

extern "C"
{
    /**
     * @brief 创建新的原生宿主实例
     * @param[out] out_handle 用于存储结果宿主句柄的指针
     * @return NativePluginHostStatus 表示成功或特定错误
     */
    NATIVE_PLUGIN_HOST_API NativePluginHostStatus native_plugin_host_create(native_host_handle_t *out_handle);

    /**
     * @brief 销毁原生宿主实例并清理其所有插件
     * @param handle 要销毁的宿主实例句柄
     * @return NativePluginHostStatus 表示成功或特定错误
     */
    NATIVE_PLUGIN_HOST_API NativePluginHostStatus native_plugin_host_destroy(native_host_handle_t handle);

    /**
     * @brief 使用指定的运行时配置将插件加载到宿主中
     * @param host_handle 宿主实例句柄
     * @param runtime_config_path 运行时配置JSON文件的路径
     * @param[out] out_plugin_handle 用于存储结果插件句柄的指针
     * @return NativePluginHostStatus 表示成功或特定错误
     */
    NATIVE_PLUGIN_HOST_API NativePluginHostStatus native_plugin_host_load(
        native_host_handle_t host_handle,
        const char *runtime_config_path,
        native_plugin_handle_t *out_plugin_handle);

    /**
     * @brief 从宿主中卸载插件并清理其资源
     * @param host_handle 宿主实例句柄
     * @param plugin_handle 要卸载的插件句柄
     * @return NativePluginHostStatus 表示成功或特定错误
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