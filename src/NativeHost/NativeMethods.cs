using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

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

    [LibraryImport(LibraryName, EntryPoint = "create")]
    internal static partial NativeHostStatus Create(out IntPtr handle);

    [LibraryImport(LibraryName, EntryPoint = "destroy")]
    internal static partial NativeHostStatus Destroy(IntPtr handle);

    [LibraryImport(LibraryName, EntryPoint = "initialize")]
    internal static partial NativeHostStatus Initialize(IntPtr handle);

    [LibraryImport(LibraryName, EntryPoint = "load", StringMarshalling = StringMarshalling.Utf8)]
    internal static partial NativeHostStatus Load(IntPtr handle,
        string assemblyPath,
        out IntPtr assemblyHandle);

    [LibraryImport(LibraryName, EntryPoint = "unload")]
    internal static partial NativeHostStatus Unload(IntPtr handle, IntPtr assemblyHandle);

    [LibraryImport(LibraryName, EntryPoint = "get_function_pointer", StringMarshalling = StringMarshalling.Utf8)]
    internal static partial NativeHostStatus GetFunctionPointer(
        IntPtr handle,
        IntPtr assemblyHandle,
        string typeName,
        string methodName,
        out IntPtr functionPointer);

    [LibraryImport(LibraryName, EntryPoint = "get_delegate", StringMarshalling = StringMarshalling.Utf8)]
    internal static partial NativeHostStatus GetDelegate(
        IntPtr handle,
        IntPtr assemblyHandle,
        string typeName,
        string methodName,
        out IntPtr delegatePointer);

    [LibraryImport(LibraryName, EntryPoint = "invoke_method", StringMarshalling = StringMarshalling.Utf8)]
    internal static partial NativeHostStatus InvokeMethod(
        IntPtr handle,
        IntPtr assemblyHandle,
        string typeName,
        string methodName,
        IntPtr[] args,
        int argCount,
        out IntPtr result);
}