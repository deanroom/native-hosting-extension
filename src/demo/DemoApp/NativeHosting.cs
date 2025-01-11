using System.Runtime.InteropServices;

namespace DemoApp;

public partial class NativeHosting : IDisposable
{
    private const string LibraryName = "NativeHosting";
    private bool _isInitialized;
    private bool _isDisposed;

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8, EntryPoint = "initialize_runtime")]
    [return: MarshalAs(UnmanagedType.I1)]
    private static partial bool InitializeRuntime(string runtimeConfigPath);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8, EntryPoint = "load_assembly_and_get_function_pointer")]
    private static partial IntPtr LoadAssemblyAndGetFunctionPointer(
        string assemblyPath,
        string typeName,
        string methodName,
        string delegateTypeName);

    [LibraryImport(LibraryName, EntryPoint = "close_runtime")]
    private static partial void CloseRuntime();

    public void Initialize(string runtimeConfigPath)
    {
        if (_isInitialized)
        {
            throw new InvalidOperationException("Runtime is already initialized");
        }

        if (!InitializeRuntime(runtimeConfigPath))
        {
            throw new InvalidOperationException("Failed to initialize runtime");
        }

        _isInitialized = true;
    }

    public T GetFunction<T>(string assemblyPath, string typeName, string methodName) where T : Delegate
    {
        if (!_isInitialized)
        {
            throw new InvalidOperationException("Runtime is not initialized");
        }

        var functionPtr = LoadAssemblyAndGetFunctionPointer(
            assemblyPath,
            typeName,
            methodName,
            typeof(T).ToString());

        if (functionPtr == IntPtr.Zero)
        {
            throw new InvalidOperationException($"Failed to load method {methodName}");
        }

        return Marshal.GetDelegateForFunctionPointer<T>(functionPtr);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!_isDisposed)
        {
            if (disposing && _isInitialized)
            {
                CloseRuntime();
                _isInitialized = false;
            }

            _isDisposed = true;
        }
    }

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    ~NativeHosting()
    {
        Dispose(false);
    }
} 