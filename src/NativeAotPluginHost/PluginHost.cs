using System.Runtime.InteropServices;
using Microsoft.Extensions.Logging;

namespace NativeAotPluginHost;


/// <summary>
/// 原生 AOT 插件宿主库,这是 native_aot_plugin_host 库的 .NET 包装器,
/// 用于在 AOT 编译的 .NET 应用程序中动态加载和执行托管程序集中的方法。
/// </summary>
public partial class PluginHost : IDisposable
{
    private const string LibraryName = "NativeAotPluginHost";
    private bool _isInitialized;
    private bool _isDisposed;

    public PluginHost()
    {
    }

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

    ~PluginHost()
    {
        Dispose(false);
    }
}