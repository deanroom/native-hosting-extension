using System.Runtime.InteropServices;

namespace NativeHost;

/// <summary>
/// Native host API status codes that match the C/C++ definitions
/// </summary>
public enum NativeHostStatus
{
    Success = 0,
    ErrorHostNotFound = -100,
    ErrorHostAlreadyExists = -101,
    ErrorAssemblyNotFound = -200,
    ErrorAssemblyNotInitialized = -203,
    ErrorRuntimeInit = -300,
    ErrorHostfxrNotFound = -302,
    ErrorDelegateNotFound = -303,
    ErrorAssemblyLoad = -400,
    ErrorTypeLoad = -401,
    ErrorMethodLoad = -402,
    ErrorInvalidArg = -500
}

/// <summary>
/// Native methods imported from the native_host library
/// </summary>
internal static partial class NativeMethods
{
    private const string LibraryName = "native_host";

    [LibraryImport(LibraryName, EntryPoint = "native_host_create")]
    internal static partial NativeHostStatus Create(out IntPtr handle);

    [LibraryImport(LibraryName, EntryPoint = "native_host_destroy")]
    internal static partial NativeHostStatus Destroy(IntPtr handle);

    [LibraryImport(LibraryName, EntryPoint = "native_host_initialize")]
    internal static partial NativeHostStatus Initialize(IntPtr handle);

    [LibraryImport(LibraryName, EntryPoint = "native_host_load_assembly", StringMarshalling = StringMarshalling.Utf8)]
    internal static partial NativeHostStatus Load(IntPtr handle,
        string assemblyPath,
        out IntPtr assemblyHandle);

    [LibraryImport(LibraryName, EntryPoint = "native_host_unload_assembly")]
    internal static partial NativeHostStatus Unload(IntPtr handle, IntPtr assemblyHandle);

    [LibraryImport(LibraryName, EntryPoint = "native_host_get_delegate", StringMarshalling = StringMarshalling.Utf8)]
    internal static partial NativeHostStatus GetFunctionPointer(
        IntPtr handle,
        IntPtr assemblyHandle,
        string typeName,
        string methodName,
        out IntPtr functionPointer);
}