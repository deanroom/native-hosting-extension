using System.Runtime.InteropServices;

namespace NativePluginHost;

/// <summary>
/// Native Plugin Host 的托管侧包装器，提供了与原生 API 对应的托管方法。
/// 用于在原生应用程序中动态加载和执行托管程序集中的方法。
/// </summary>
public class PluginHost : IDisposable
{
    private bool _isDisposed;
    private IntPtr _handle;
    private readonly Dictionary<IntPtr, Plugin> _plugins;

    public PluginHost()
    {
        _plugins = new Dictionary<IntPtr, Plugin>();
        var result = NativeMethods.CreateHost(out _handle);
        if (result < 0)
        {
            ThrowForError(result, "Failed to create host instance");
        }
    }

    /// <summary>
    /// 加载插件
    /// </summary>
    /// <param name="runtimeConfigPath">运行时配置文件路径</param>
    /// <returns>插件实例</returns>
    /// <exception cref="ArgumentException">配置文件路径无效</exception>
    /// <exception cref="InvalidOperationException">运行时已初始化或其他运行时错误</exception>
    /// <exception cref="DllNotFoundException">找不到 hostfxr 库</exception>
    /// <exception cref="TypeInitializationException">运行时初始化失败</exception>
    public Plugin LoadPlugin(string runtimeConfigPath)
    {
        if (string.IsNullOrEmpty(runtimeConfigPath))
        {
            throw new ArgumentException("Runtime config path cannot be null or empty", nameof(runtimeConfigPath));
        }

        var result = NativeMethods.LoadPlugin(_handle, runtimeConfigPath, out var pluginHandle);
        if (result < 0)
        {
            ThrowForError(result, runtimeConfigPath);
        }

        var plugin = new Plugin(this, pluginHandle, runtimeConfigPath);
        _plugins[pluginHandle] = plugin;
        return plugin;
    }

    /// <summary>
    /// 卸载插件
    /// </summary>
    /// <param name="pluginHandle">插件句柄</param>
    /// <exception cref="ArgumentException">插件句柄无效</exception>
    /// <exception cref="InvalidOperationException">插件未找到或其他错误</exception>
    internal void UnloadPlugin(IntPtr pluginHandle)
    {
        if (pluginHandle == IntPtr.Zero)
        {
            throw new ArgumentException("Plugin handle cannot be zero", nameof(pluginHandle));
        }

        if (!_plugins.ContainsKey(pluginHandle))
        {
            throw new InvalidOperationException($"Plugin not found: {pluginHandle}");
        }

        var result = NativeMethods.UnloadPlugin(_handle, pluginHandle);
        if (result < 0)
        {
            ThrowForError(result, $"Failed to unload plugin: {pluginHandle}");
        }

        _plugins.Remove(pluginHandle);
    }

    /// <summary>
    /// 获取函数委托
    /// </summary>
    /// <typeparam name="T">委托类型</typeparam>
    /// <param name="pluginHandle">插件句柄</param>
    /// <param name="assemblyPath">程序集路径</param>
    /// <param name="typeName">类型名称</param>
    /// <param name="methodName">方法名称</param>
    /// <returns>函数委托</returns>
    internal T GetFunction<T>(IntPtr pluginHandle, string assemblyPath, string typeName, string methodName) where T : Delegate
    {
        if (pluginHandle == IntPtr.Zero)
        {
            throw new ArgumentException("Plugin handle cannot be zero", nameof(pluginHandle));
        }

        if (!_plugins.ContainsKey(pluginHandle))
        {
            throw new InvalidOperationException($"Plugin not found: {pluginHandle}");
        }

        if (string.IsNullOrEmpty(assemblyPath))
            throw new ArgumentException("Assembly path cannot be null or empty", nameof(assemblyPath));
        if (string.IsNullOrEmpty(typeName))
            throw new ArgumentException("Type name cannot be null or empty", nameof(typeName));
        if (string.IsNullOrEmpty(methodName))
            throw new ArgumentException("Method name cannot be null or empty", nameof(methodName));

        var result = NativeMethods.GetFunctionPointer(
            _handle,
            pluginHandle,
            assemblyPath,
            typeName,
            methodName,
            typeof(T).ToString(),
            out var functionPtr);

        if (result < 0)
        {
            ThrowForError(result, $"{assemblyPath}:{typeName}.{methodName}");
        }

        return Marshal.GetDelegateForFunctionPointer<T>(functionPtr);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!_isDisposed)
        {
            if (disposing)
            {
                foreach (var plugin in _plugins.Values.ToList())
                {
                    plugin.Dispose();
                }
                _plugins.Clear();
            }

            if (_handle != IntPtr.Zero)
            {
                NativeMethods.DestroyHost(_handle);
                _handle = IntPtr.Zero;
            }

            _isDisposed = true;
        }
    }

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    private static void ThrowForError(int errorCode, string context)
    {
        switch (errorCode)
        {
            case NativeMethods.ErrorHostNotFound:
                throw new InvalidOperationException($"Host not found: {context}");
            case NativeMethods.ErrorPluginNotFound:
                throw new InvalidOperationException($"Plugin not found: {context}");
            case NativeMethods.ErrorPluginNotInitialized:
                throw new InvalidOperationException($"Plugin not initialized: {context}");
            case NativeMethods.ErrorRuntimeInit:
                throw new TypeInitializationException($"Runtime initialization failed: {context}", null);
            case NativeMethods.ErrorHostfxrNotFound:
                throw new DllNotFoundException($"hostfxr not found: {context}");
            case NativeMethods.ErrorAssemblyLoad:
                throw new BadImageFormatException($"Failed to load assembly: {context}");
            case NativeMethods.ErrorTypeLoad:
                throw new TypeLoadException($"Failed to load type: {context}");
            case NativeMethods.ErrorMethodLoad:
                throw new MissingMethodException($"Failed to load method: {context}");
            case NativeMethods.ErrorInvalidArg:
                throw new ArgumentException($"Invalid argument: {context}");
            default:
                throw new InvalidOperationException($"Unknown error {errorCode}: {context}");
        }
    }
}