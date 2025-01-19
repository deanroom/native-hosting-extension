/**
 * @file native_host.h
 * @brief 本机加载 .NET 程序集的接口定义
 *
 * 本头文件定义了一个可以加载和交互.NET 程序集的接口。
 * 设计遵循以下关键原则：
 *
 * 1. 基于句柄的API：所有操作都通过不透明句柄执行，确保安全性
 * 2. 线程安全操作：所有公共API都通过内部同步机制保护
 * 3. 资源管理：显式的创建/销毁生命周期，确保正确清理
 * 4. 错误处理：详细的状态码，实现健壮的错误处理
 */

#pragma once

// 平台特定的DLL导出/导入宏
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
extern "C"
{
#endif

    /**
     * @brief 本机托管API状态码
     *
     * 这些状态码按以下类别组织：
     * - 主机相关错误 (-100 到 -199)
     * - 程序集相关错误 (-200 到 -299)
     * - 运行时相关错误 (-300 到 -399)
     * - 加载相关错误 (-400 到 -499)
     * - 通用错误 (-500 到 -599)
     */
    enum NativeHostStatus
    {
        SUCCESS = 0,
        ERROR_HOST_NOT_FOUND = -100,           ///< 未找到指定的主机句柄
        ERROR_HOST_ALREADY_EXISTS = -101,      ///< 尝试创建主机时发现已存在
        ERROR_ASSEMBLY_NOT_FOUND = -200,       ///< 未找到指定的程序集句柄
        ERROR_ASSEMBLY_NOT_INITIALIZED = -203, ///< 在初始化之前尝试使用程序集
        ERROR_RUNTIME_INIT = -300,             ///< .NET运行时初始化失败
        ERROR_HOSTFXR_NOT_FOUND = -302,        ///< 无法找到或加载.NET主机解析器
        ERROR_DELEGATE_NOT_FOUND = -303,       ///< 获取指定方法的委托失败
        ERROR_ASSEMBLY_LOAD = -400,            ///< 加载指定程序集失败
        ERROR_TYPE_LOAD = -401,                ///< 加载指定类型失败
        ERROR_METHOD_LOAD = -402,              ///< 加载指定方法失败
        ERROR_INVALID_ARG = -500               ///< 提供了无效参数
    };

    /**
     * @brief 主机和程序集实例的不透明句柄类型
     *
     * 这些句柄用于维护类型安全性，同时隐藏实现细节。
     * 它们表示由本机主机管理的内部对象的引用。
     */
    typedef void *native_handle_t;
    typedef native_handle_t native_host_handle_t;     ///< 本机主机实例的句柄
    typedef native_handle_t native_assembly_handle_t; ///< 已加载程序集的句柄

    /**
     * @brief 创建新的本机主机实例
     *
     * 必须在执行任何其他操作之前调用此函数。
     * 进程中同一时间只能存在一个主机。
     *
     * @param[out] handle 接收主机句柄的指针
     * @return NativeHostStatus 表示成功或失败的状态码
     */
    NATIVE_HOST_API enum NativeHostStatus native_host_create(/*out*/ native_host_handle_t *handle);

    /**
     * @brief 销毁本机主机实例
     *
     * 此函数清理与主机相关的所有资源，包括已加载的程序集。
     * 调用后句柄将变为无效。
     *
     * @param handle 要销毁的主机实例句柄
     * @return NativeHostStatus 表示成功或失败的状态码
     */
    NATIVE_HOST_API enum NativeHostStatus native_host_destroy(native_host_handle_t handle);

    /**
     * @brief 初始化主机的.NET运行时
     *
     * 必须在创建之后、加载任何程序集之前调用此函数。
     * 使用默认配置设置.NET运行时环境。
     *
     * @param handle 主机实例句柄
     * @return NativeHostStatus 表示成功或失败的状态码
     */
    NATIVE_HOST_API enum NativeHostStatus native_host_initialize(native_host_handle_t handle);

    /**
     * @brief 将.NET程序集加载到主机中
     *
     * 从指定路径加载程序集，并保持加载状态直到显式卸载或主机被销毁。
     *
     * @param handle 主机实例句柄
     * @param assembly_path 要加载的程序集文件路径
     * @param[out] assembly_handle 接收程序集句柄的指针
     * @return NativeHostStatus 表示成功或失败的状态码
     */
    NATIVE_HOST_API enum NativeHostStatus native_host_load_assembly(
        native_host_handle_t handle,
        const char *assembly_path,
        /*out*/ native_assembly_handle_t *assembly_handle);

    /**
     * @brief 卸载之前加载的程序集
     *
     * 此函数卸载指定的程序集并使其句柄无效。
     * 从该程序集获取的所有委托都将变为无效。
     *
     * @param handle 主机实例句柄
     * @param assembly_handle 要卸载的程序集句柄
     * @return NativeHostStatus 表示成功或失败的状态码
     */
    NATIVE_HOST_API enum NativeHostStatus native_host_unload_assembly(
        native_host_handle_t handle,
        native_assembly_handle_t assembly_handle);

    /**
     * @brief 从已加载的程序集获取函数委托
     *
     * 此函数在指定的程序集中查找方法，并返回一个可从本机代码调用的函数指针。
     *
     * @param handle 主机实例句柄
     * @param assembly_handle 已加载程序集的句柄
     * @param type_name 包含方法的类型的完全限定名
     * @param method_name 要获取委托的方法名
     * @param[out] delegate 接收函数指针的指针
     * @return NativeHostStatus 表示成功或失败的状态码
     */
    NATIVE_HOST_API enum NativeHostStatus native_host_get_delegate(
        native_host_handle_t handle,
        native_assembly_handle_t assembly_handle,
        const char *type_name,
        const char *method_name,
        void **delegate);

#ifdef __cplusplus
}
#endif