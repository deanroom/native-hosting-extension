using System.Runtime.InteropServices;
namespace NativeHost;

/// <summary>
/// Native host for managing .NET assemblies in native applications
/// </summary>
public sealed class NativeHost : IDisposable
{
    private bool _isDisposed;
    private readonly IntPtr _handle;
    private readonly Dictionary<IntPtr, Assembly> _assemblies;

    public NativeHost()
    {
        _assemblies = new Dictionary<IntPtr, Assembly>();

        var status = NativeMethods.Create(out _handle);
        if (status != NativeHostStatus.Success)
        {
            ThrowForStatus(status, "Failed to create host instance");
        }

        status = NativeMethods.Initialize(_handle);
        if (status != NativeHostStatus.Success)
        {
            ThrowForStatus(status, "Failed to initialize runtime");
        }
    }

    /// <summary>
    /// Load a .NET assembly into the host
    /// </summary>
    /// <param name="assemblyPath">Path to the .NET assembly file</param>
    /// <returns>Assembly instance</returns>
    public Assembly Load(string assemblyPath)
    {
        ThrowIfDisposed();

        if (string.IsNullOrEmpty(assemblyPath))
        {
            throw new ArgumentException("Assembly path cannot be null or empty", nameof(assemblyPath));
        }

        var status = NativeMethods.Load(_handle, assemblyPath, out var assemblyHandle);
        if (status != NativeHostStatus.Success)
        {
            ThrowForStatus(status, $"Failed to load assembly: {assemblyPath}");
        }

        var assembly = new Assembly(this, assemblyHandle, assemblyPath);
        _assemblies[assemblyHandle] = assembly;
        return assembly;
    }

    /// <summary>
    /// Unload an assembly from the host
    /// </summary>
    internal void Unload(Assembly assembly)
    {
        ThrowIfDisposed();

        if (assembly == null)
        {
            throw new ArgumentNullException(nameof(assembly));
        }

        if (!_assemblies.ContainsKey(assembly.Handle))
        {
            throw new ArgumentException("Assembly was not loaded by this host instance", nameof(assembly));
        }

        var status = NativeMethods.Unload(_handle, assembly.Handle);
        if (status != NativeHostStatus.Success)
        {
            ThrowForStatus(status, $"Failed to unload assembly: {assembly.AssemblyPath}");
        }

        _assemblies.Remove(assembly.Handle);
    }

    /// <summary>
    /// Get a function pointer to a specific method
    /// </summary>
    internal T GetFunction<T>(IntPtr assemblyHandle, string typeName, string methodName) where T : Delegate
    {
        ThrowIfDisposed();

        if (!_assemblies.ContainsKey(assemblyHandle))
        {
            throw new ArgumentException("Assembly handle is not valid", nameof(assemblyHandle));
        }

        if (string.IsNullOrEmpty(typeName))
        {
            throw new ArgumentException("Type name cannot be null or empty", nameof(typeName));
        }

        if (string.IsNullOrEmpty(methodName))
        {
            throw new ArgumentException("Method name cannot be null or empty", nameof(methodName));
        }

        var status = NativeMethods.GetFunctionPointer(
            _handle,
            assemblyHandle,
            typeName,
            methodName,
            out var functionPtr);

        if (status != NativeHostStatus.Success)
        {
            ThrowForStatus(status, $"Failed to get function pointer for {typeName}.{methodName}");
        }

        return Marshal.GetDelegateForFunctionPointer<T>(functionPtr);
    }

    public void Dispose()
    {
        if (!_isDisposed)
        {
            foreach (var assembly in _assemblies.Values.ToList())
            {
                assembly.Dispose();
            }
            _assemblies.Clear();

            if (_handle != IntPtr.Zero)
            {
                NativeMethods.Destroy(_handle);
            }

            _isDisposed = true;
        }
    }

    private void ThrowIfDisposed()
    {
        if (_isDisposed)
        {
            throw new ObjectDisposedException(nameof(NativeHost));
        }
    }

    private static void ThrowForStatus(NativeHostStatus status, string message)
    {
        switch (status)
        {
            case NativeHostStatus.Success:
                return;
            case NativeHostStatus.ErrorHostNotFound:
            case NativeHostStatus.ErrorHostAlreadyExists:
            case NativeHostStatus.ErrorAssemblyNotFound:
            case NativeHostStatus.ErrorAssemblyNotInitialized:
                throw new InvalidOperationException(message);
            case NativeHostStatus.ErrorRuntimeInit:
                throw new TypeInitializationException(typeof(NativeHost).FullName, new InvalidOperationException(message));
            case NativeHostStatus.ErrorHostfxrNotFound:
                throw new DllNotFoundException(message);
            case NativeHostStatus.ErrorDelegateNotFound:
                throw new MissingMethodException(message);
            case NativeHostStatus.ErrorAssemblyLoad:
                throw new BadImageFormatException(message);
            case NativeHostStatus.ErrorTypeLoad:
                throw new TypeLoadException(message);
            case NativeHostStatus.ErrorMethodLoad:
                throw new MissingMethodException(message);
            case NativeHostStatus.ErrorInvalidArg:
                throw new ArgumentException(message);
            default:
                throw new InvalidOperationException($"Unknown error {status}: {message}");
        }
    }
}

/// <summary>
/// Represents a loaded assembly instance
/// </summary>
public sealed class Assembly : IDisposable
{
    private readonly NativeHost _host;
    private readonly Dictionary<string, Delegate> _cachedDelegates;
    private bool _isDisposed;

    public IntPtr Handle { get; }
    public string AssemblyPath { get; }

    internal Assembly(NativeHost host, IntPtr handle, string assemblyPath)
    {
        _host = host ?? throw new ArgumentNullException(nameof(host));
        Handle = handle;
        AssemblyPath = assemblyPath;
        _cachedDelegates = new Dictionary<string, Delegate>();
    }

    /// <summary>
    /// Get a function pointer to a specific method
    /// </summary>
    public T GetFunction<T>(string typeName, string methodName) where T : Delegate
    {
        ThrowIfDisposed();

        var key = $"{AssemblyPath}:{typeName}.{methodName}";
        if (_cachedDelegates.TryGetValue(key, out var cachedDelegate))
        {
            return (T)cachedDelegate;
        }

        var function = _host.GetFunction<T>(Handle, typeName, methodName);
        _cachedDelegates[key] = function;
        return function;
    }

    /// <summary>
    /// Clear the delegate cache
    /// </summary>
    public void ClearCache()
    {
        ThrowIfDisposed();
        _cachedDelegates.Clear();
    }

    public void Dispose()
    {
        if (!_isDisposed)
        {
            _cachedDelegates.Clear();
            _host.Unload(this);
            _isDisposed = true;
        }
    }

    private void ThrowIfDisposed()
    {
        if (_isDisposed)
        {
            throw new ObjectDisposedException(nameof(Assembly));
        }
    }
}