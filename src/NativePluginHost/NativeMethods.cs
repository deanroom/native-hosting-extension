using System.Runtime.InteropServices;

namespace NativePluginHost;

/// <summary>
/// 提供对原生 Plugin Host API 的访问
/// </summary>
internal static partial class NativeMethods
{
    private const string LibraryName = "native_plugin_host";

    // 成功代码（0和正值）
    internal const int Success = 0;

    // 宿主错误（-100到-199）
    internal const int ErrorHostNotFound = -100;

    // 插件错误（-200到-299）
    internal const int ErrorPluginNotFound = -200;
    internal const int ErrorPluginNotInitialized = -203;

    // 运行时错误（-300到-399）
    internal const int ErrorRuntimeInit = -300;
    internal const int ErrorHostfxrNotFound = -302;
    internal const int ErrorDelegateNotFound = -303;

    // 程序集错误（-400到-499）
    internal const int ErrorAssemblyLoad = -400;
    internal const int ErrorTypeLoad = -401;
    internal const int ErrorMethodLoad = -402;

    // 通用错误（-500到-599）
    internal const int ErrorInvalidArg = -500;

    /// <summary>
    /// 创建新的原生宿主实例
    /// </summary>
    [LibraryImport(LibraryName, EntryPoint = "native_plugin_host_create")]
    internal static partial int CreateHost(out IntPtr handle);

    /// <summary>
    /// 加载插件到宿主中
    /// </summary>
    [LibraryImport(LibraryName, EntryPoint = "native_plugin_host_load", StringMarshalling = StringMarshalling.Utf8)]
    internal static partial int LoadPlugin(IntPtr handle, string runtimeConfigPath, out IntPtr pluginHandle);

    /// <summary>
    /// 获取托管方法的函数指针
    /// </summary>
    [LibraryImport(LibraryName, EntryPoint = "native_plugin_host_get_function_pointer", StringMarshalling = StringMarshalling.Utf8)]
    internal static partial int GetFunctionPointer(
        IntPtr handle,
        IntPtr pluginHandle,
        string assemblyPath,
        string typeName,
        string methodName,
        string delegateTypeName,
        out IntPtr functionPtr);

    /// <summary>
    /// 从宿主中卸载插件
    /// </summary>
    [LibraryImport(LibraryName, EntryPoint = "native_plugin_host_unload")]
    internal static partial int UnloadPlugin(IntPtr handle, IntPtr pluginHandle);

    /// <summary>
    /// 销毁原生宿主实例
    /// </summary>
    [LibraryImport(LibraryName, EntryPoint = "native_plugin_host_destroy")]
    internal static partial int DestroyHost(IntPtr handle);
}