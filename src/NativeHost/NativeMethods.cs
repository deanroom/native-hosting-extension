using System.Runtime.InteropServices;

namespace NativeHost;

/// <summary>
/// 提供对原生 Plugin Host API 的访问
/// </summary>
internal static partial class NativeMethods
{
    private const string LibraryName = "native_host";

    // Success code
    public const int Success = 0;

    // Host errors (-100 to -199)
    public const int ErrorHostNotFound = -100;
    public const int ErrorHostAlreadyExists = -101;

    // Assembly errors (-200 to -299)
    public const int ErrorAssemblyNotFound = -200;
    public const int ErrorAssemblyNotInitialized = -203;

    // Runtime errors (-300 to -399)
    public const int ErrorRuntimeInit = -300;
    public const int ErrorHostfxrNotFound = -302;
    public const int ErrorDelegateNotFound = -303;

    // Assembly errors (-400 to -499)
    public const int ErrorAssemblyLoad = -400;
    public const int ErrorTypeLoad = -401;
    public const int ErrorMethodLoad = -402;

    // Argument errors (-500 to -599)
    public const int ErrorInvalidArg = -500;

    /// <summary>
    /// 创建新的原生宿主实例
    /// </summary>
    [DllImport(LibraryName, EntryPoint = "create", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int Create(out IntPtr hostHandle);

    /// <summary>
    /// 销毁原生宿主实例
    /// </summary>
    [DllImport(LibraryName, EntryPoint = "destroy", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int Destroy(IntPtr hostHandle);

    /// <summary>
    /// 初始化运行时
    /// </summary>
    [DllImport(LibraryName, EntryPoint = "initialize_runtime", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int InitializeRuntime(IntPtr hostHandle);

    /// <summary>
    /// 加载插件到宿主中
    /// </summary>
    [DllImport(LibraryName, EntryPoint = "load", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int LoadAssembly(IntPtr hostHandle, string assemblyPath, out IntPtr assemblyHandle);

    /// <summary>
    /// 从宿主中卸载插件
    /// </summary>
    [DllImport(LibraryName, EntryPoint = "unload", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int UnloadAssembly(IntPtr hostHandle, IntPtr assemblyHandle);

    /// <summary>
    /// 获取托管方法的函数指针
    /// </summary>
    [DllImport(LibraryName, EntryPoint = "get_function_pointer", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int GetFunctionPointer(
        IntPtr hostHandle,
        IntPtr assemblyHandle,
        string typeName,
        string methodName,
        out IntPtr functionPointer);
}