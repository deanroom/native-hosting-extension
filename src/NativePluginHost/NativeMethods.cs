using System.Runtime.InteropServices;

namespace NativePluginHost;

internal static partial class NativeMethods
{
    private const string LibraryName = "native_plugin_host";

    // Success codes (0 and positive values)
    internal const int Success = 0;

    // Host errors (-100 to -199)
    internal const int ErrorHostNotFound = -100;

    // Plugin errors (-200 to -299)
    internal const int ErrorPluginNotFound = -200;
    internal const int ErrorPluginNotInitialized = -203;

    // Runtime errors (-300 to -399)
    internal const int ErrorRuntimeInit = -300;
    internal const int ErrorHostfxrNotFound = -302;
    internal const int ErrorDelegateNotFound = -303;

    // Assembly errors (-400 to -499)
    internal const int ErrorAssemblyLoad = -400;
    internal const int ErrorTypeLoad = -401;
    internal const int ErrorMethodLoad = -402;

    // General errors (-500 to -599)
    internal const int ErrorInvalidArg = -500;

    [LibraryImport(LibraryName, EntryPoint = "native_plugin_host_create")]
    internal static partial int CreateHost(out IntPtr handle);

    [LibraryImport(LibraryName, EntryPoint = "native_plugin_host_load", StringMarshalling = StringMarshalling.Utf8)]
    internal static partial int LoadPlugin(IntPtr handle, string runtimeConfigPath, out IntPtr pluginHandle);

    [LibraryImport(LibraryName, EntryPoint = "native_plugin_host_get_function_pointer", StringMarshalling = StringMarshalling.Utf8)]
    internal static partial int GetFunctionPointer(
        IntPtr handle,
        IntPtr pluginHandle,
        string assemblyPath,
        string typeName,
        string methodName,
        string delegateTypeName,
        out IntPtr functionPtr);

    [LibraryImport(LibraryName, EntryPoint = "native_plugin_host_unload")]
    internal static partial int UnloadPlugin(IntPtr handle, IntPtr pluginHandle);

    [LibraryImport(LibraryName, EntryPoint = "native_plugin_host_destroy")]
    internal static partial int DestroyHost(IntPtr handle);
}