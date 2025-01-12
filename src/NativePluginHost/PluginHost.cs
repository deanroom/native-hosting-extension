using System.Runtime.InteropServices;
using Microsoft.Extensions.Logging;

namespace NativePluginHost;

/// <summary>
/// 原生 AOT 插件宿主库,这是 native_aot_plugin_host 库的 .NET 包装器,
/// 用于在 AOT 编译的 .NET 应用程序中动态加载和执行托管程序集中的方法。
/// </summary>
public partial class PluginHost : IDisposable
{
    private bool _isDisposed;
    private IntPtr _handle;
    private IntPtr _pluginHandle;

    public PluginHost()
    {
        var result = NativeMethods.CreateHost(out _handle);
        if (result < 0)
        {
            ThrowForError(result, "Failed to create host instance");
        }
    }

    /// <summary>
    /// 初始化运行时
    /// </summary>
    /// <param name="runtimeConfigPath">运行时配置文件路径</param>
    /// <exception cref="ArgumentException">配置文件路径无效</exception>
    /// <exception cref="InvalidOperationException">运行时已初始化或其他运行时错误</exception>
    /// <exception cref="DllNotFoundException">找不到 hostfxr 库</exception>
    /// <exception cref="TypeInitializationException">运行时初始化失败</exception>
    public void Initialize(string runtimeConfigPath)
    {
        if (string.IsNullOrEmpty(runtimeConfigPath))
        {
            throw new ArgumentException("Runtime config path cannot be null or empty", nameof(runtimeConfigPath));
        }

        var result = NativeMethods.LoadPlugin(_handle, runtimeConfigPath, out _pluginHandle);
        if (result < 0)
        {
            ThrowForError(result, runtimeConfigPath);
        }
    }

    /// <summary>
    /// 获取函数委托
    /// </summary>
    /// <typeparam name="T">委托类型</typeparam>
    /// <param name="assemblyPath">程序集路径</param>
    /// <param name="typeName">类型名称</param>
    /// <param name="methodName">方法名称</param>
    /// <returns>函数委托</returns>
    /// <exception cref="InvalidOperationException">运行时未初始化</exception>
    /// <exception cref="ArgumentException">参数无效</exception>
    /// <exception cref="BadImageFormatException">程序集加载失败</exception>
    /// <exception cref="TypeLoadException">类型加载失败</exception>
    /// <exception cref="MissingMethodException">方法不存在</exception>
    public T GetFunction<T>(string assemblyPath, string typeName, string methodName) where T : Delegate
    {
        if (_pluginHandle == IntPtr.Zero)
        {
            throw new InvalidOperationException("Plugin is not initialized");
        }

        if (string.IsNullOrEmpty(assemblyPath))
            throw new ArgumentException("Assembly path cannot be null or empty", nameof(assemblyPath));
        if (string.IsNullOrEmpty(typeName))
            throw new ArgumentException("Type name cannot be null or empty", nameof(typeName));
        if (string.IsNullOrEmpty(methodName))
            throw new ArgumentException("Method name cannot be null or empty", nameof(methodName));

        var result = NativeMethods.GetFunctionPointer(
            _handle,
            _pluginHandle,
            assemblyPath,
            typeName,
            methodName,
            typeof(T).ToString(),
            out var functionPtr);

        if (result < 0)
        {
            ThrowForError(result, $"{assemblyPath}:{typeName}.{methodName}");
        }

        if (functionPtr == IntPtr.Zero)
        {
            throw new InvalidOperationException($"Failed to get function pointer for {methodName}");
        }

        return Marshal.GetDelegateForFunctionPointer<T>(functionPtr);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!_isDisposed)
        {
            if (disposing)
            {
                if (_pluginHandle != IntPtr.Zero)
                {
                    var result = NativeMethods.UnloadPlugin(_handle, _pluginHandle);
                    if (result < 0)
                    {
                        ThrowForError(result, "Failed to unload plugin");
                    }
                    _pluginHandle = IntPtr.Zero;
                }
                if (_handle != IntPtr.Zero)
                {
                    var result = NativeMethods.DestroyHost(_handle);
                    if (result < 0)
                    {
                        ThrowForError(result, "Failed to destroy host");
                    }
                    _handle = IntPtr.Zero;
                }
            }

            _isDisposed = true;
        }
    }

    /// <summary>
    /// 将原生错误码转换为异常
    /// </summary>
    private void ThrowForError(int errorCode, string context)
    {
        if (errorCode >= 0) return; // Success codes

        switch (errorCode)
        {
            // Host errors
            case NativeMethods.ErrorHostNotFound:
                throw new InvalidOperationException($"Host not found: {context}");

            // Plugin errors
            case NativeMethods.ErrorPluginNotFound:
                throw new InvalidOperationException($"Plugin not found: {context}");
            case NativeMethods.ErrorPluginNotInitialized:
                throw new InvalidOperationException($"Plugin is not initialized: {context}");

            // Runtime errors
            case NativeMethods.ErrorRuntimeInit:
                throw new TypeInitializationException(".NET Runtime",
                    new Exception($"Failed to initialize .NET runtime: {context}"));
            case NativeMethods.ErrorHostfxrNotFound:
                throw new DllNotFoundException("Failed to find or load hostfxr library");
            case NativeMethods.ErrorDelegateNotFound:
                throw new EntryPointNotFoundException("Failed to get required function delegate");

            // Assembly errors
            case NativeMethods.ErrorAssemblyLoad:
                throw new BadImageFormatException($"Failed to load assembly: {context}");
            case NativeMethods.ErrorTypeLoad:
                throw new TypeLoadException($"Failed to load type: {context}");
            case NativeMethods.ErrorMethodLoad:
                throw new MissingMethodException($"Failed to load method: {context}");

            // General errors
            case NativeMethods.ErrorInvalidArg:
                throw new ArgumentException($"Invalid argument provided: {context}");

            default:
                throw new InvalidOperationException($"Unknown error occurred ({errorCode}): {context}");
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