using System.Runtime.InteropServices;

namespace NativePluginHost;

/// <summary>
/// 表示一个已加载的插件实例，对应原生的 NativePlugin
/// </summary>
public class Plugin : IDisposable
{
    private readonly PluginHost _host;
    private readonly IntPtr _handle;
    private readonly string _runtimeConfigPath;
    private readonly Dictionary<string, Delegate> _cachedDelegates;
    private bool _isDisposed;

    internal Plugin(PluginHost host, IntPtr handle, string runtimeConfigPath)
    {
        _host = host ?? throw new ArgumentNullException(nameof(host));
        _handle = handle;
        _runtimeConfigPath = runtimeConfigPath;
        _cachedDelegates = new Dictionary<string, Delegate>();
    }

    /// <summary>
    /// 获取插件句柄
    /// </summary>
    public IntPtr Handle => _handle;

    /// <summary>
    /// 获取运行时配置路径
    /// </summary>
    public string RuntimeConfigPath => _runtimeConfigPath;

    /// <summary>
    /// 获取函数委托
    /// </summary>
    /// <typeparam name="T">委托类型</typeparam>
    /// <param name="assemblyPath">程序集路径</param>
    /// <param name="typeName">类型名称</param>
    /// <param name="methodName">方法名称</param>
    /// <returns>函数委托</returns>
    public T GetFunction<T>(string assemblyPath, string typeName, string methodName) where T : Delegate
    {
        ThrowIfDisposed();

        var key = $"{assemblyPath}:{typeName}.{methodName}";
        if (_cachedDelegates.TryGetValue(key, out var cachedDelegate))
        {
            return (T)cachedDelegate;
        }

        var function = _host.GetFunction<T>(_handle, assemblyPath, typeName, methodName);
        _cachedDelegates[key] = function;
        return function;
    }

    /// <summary>
    /// 清除委托缓存
    /// </summary>
    public void ClearCache()
    {
        ThrowIfDisposed();
        _cachedDelegates.Clear();
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!_isDisposed)
        {
            if (disposing)
            {
                _cachedDelegates.Clear();
                _host.UnloadPlugin(_handle);
            }
            _isDisposed = true;
        }
    }

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    private void ThrowIfDisposed()
    {
        if (_isDisposed)
        {
            throw new ObjectDisposedException(nameof(Plugin));
        }
    }
} 