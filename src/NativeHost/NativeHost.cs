using System.Runtime.InteropServices;

namespace NativeHost;

/// <summary>
/// Native host wrapper for managing .NET assemblies in native applications.
/// Provides managed methods corresponding to the native API for dynamically loading
/// and executing methods from managed assemblies.
/// </summary>
public class NativeHost : IDisposable
{
    private bool _isDisposed;
    private IntPtr _handle;
    private readonly Dictionary<IntPtr, Assembly> _assemblies;

    public NativeHost()
    {
        _assemblies = new Dictionary<IntPtr, Assembly>();
        var result = NativeMethods.Create(out _handle);
        if (result < 0)
        {
            ThrowForError(result, "Failed to create host instance");
        }

        // Initialize runtime
        result = NativeMethods.InitializeRuntime(_handle);
        if (result < 0)
        {
            ThrowForError(result, "Failed to initialize runtime");
        }
    }

    /// <summary>
    /// Load a .NET assembly into the host
    /// </summary>
    /// <param name="assemblyPath">Path to the .NET assembly file</param>
    /// <returns>Assembly instance</returns>
    /// <exception cref="ArgumentException">Invalid assembly path</exception>
    /// <exception cref="InvalidOperationException">Runtime not initialized or other runtime error</exception>
    /// <exception cref="DllNotFoundException">hostfxr library not found</exception>
    /// <exception cref="TypeInitializationException">Runtime initialization failed</exception>
    public Assembly LoadAssembly(string assemblyPath)
    {
        if (string.IsNullOrEmpty(assemblyPath))
        {
            throw new ArgumentException("Assembly path cannot be null or empty", nameof(assemblyPath));
        }

        var result = NativeMethods.LoadAssembly(_handle, assemblyPath, out var assemblyHandle);
        if (result < 0)
        {
            ThrowForError(result, assemblyPath);
        }

        var assembly = new Assembly(this, assemblyHandle, assemblyPath);
        _assemblies[assemblyHandle] = assembly;
        return assembly;
    }

    /// <summary>
    /// Unload an assembly from the host
    /// </summary>
    /// <param name="assemblyHandle">Assembly handle</param>
    /// <exception cref="ArgumentException">Invalid assembly handle</exception>
    /// <exception cref="InvalidOperationException">Assembly not found or other error</exception>
    internal void UnloadAssembly(IntPtr assemblyHandle)
    {
        if (assemblyHandle == IntPtr.Zero)
        {
            throw new ArgumentException("Assembly handle cannot be zero", nameof(assemblyHandle));
        }

        if (!_assemblies.ContainsKey(assemblyHandle))
        {
            throw new InvalidOperationException($"Assembly not found: {assemblyHandle}");
        }

        var result = NativeMethods.UnloadAssembly(_handle, assemblyHandle);
        if (result < 0)
        {
            ThrowForError(result, $"Failed to unload assembly: {assemblyHandle}");
        }

        _assemblies.Remove(assemblyHandle);
    }

    /// <summary>
    /// Get a function pointer to a specific method from a loaded assembly
    /// </summary>
    /// <typeparam name="T">Delegate type</typeparam>
    /// <param name="assemblyHandle">Assembly handle</param>
    /// <param name="assemblyPath">Assembly path</param>
    /// <param name="typeName">Fully qualified name of the type containing the method</param>
    /// <param name="methodName">Name of the method to get a pointer to</param>
    /// <returns>Function delegate</returns>
    internal T GetFunction<T>(IntPtr assemblyHandle, string typeName, string methodName) where T : Delegate
    {
        if (assemblyHandle == IntPtr.Zero)
        {
            throw new ArgumentException("Assembly handle cannot be zero", nameof(assemblyHandle));
        }

        if (!_assemblies.ContainsKey(assemblyHandle))
        {
            throw new InvalidOperationException($"Assembly not found: {assemblyHandle}");
        }

        if (string.IsNullOrEmpty(typeName))
            throw new ArgumentException("Type name cannot be null or empty", nameof(typeName));
        if (string.IsNullOrEmpty(methodName))
            throw new ArgumentException("Method name cannot be null or empty", nameof(methodName));

        var result = NativeMethods.GetFunctionPointer(
            _handle,
            assemblyHandle,
            typeName,
            methodName,
            out var functionPtr);

        if (result < 0)
        {
            ThrowForError(result, $"{typeName}.{methodName}");
        }

        return Marshal.GetDelegateForFunctionPointer<T>(functionPtr);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!_isDisposed)
        {
            if (disposing)
            {
                foreach (var assembly in _assemblies.Values.ToList())
                {
                    assembly.Dispose();
                }
                _assemblies.Clear();
            }

            if (_handle != IntPtr.Zero)
            {
                NativeMethods.Destroy(_handle);
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
            case NativeMethods.ErrorHostAlreadyExists:
                throw new InvalidOperationException($"Host already exists: {context}");
            case NativeMethods.ErrorAssemblyNotFound:
                throw new InvalidOperationException($"Assembly not found: {context}");
            case NativeMethods.ErrorAssemblyNotInitialized:
                throw new InvalidOperationException($"Assembly not initialized: {context}");
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

/// <summary>
/// Represents a loaded assembly instance, corresponding to the native Assembly class
/// </summary>
public class Assembly : IDisposable
{
    private readonly NativeHost _host;
    private readonly IntPtr _handle;
    private readonly string _assemblyPath;
    private readonly Dictionary<string, Delegate> _cachedDelegates;
    private bool _isDisposed;

    internal Assembly(NativeHost host, IntPtr handle, string assemblyPath)
    {
        _host = host ?? throw new ArgumentNullException(nameof(host));
        _handle = handle;
        _assemblyPath = assemblyPath;
        _cachedDelegates = new Dictionary<string, Delegate>();
    }

    /// <summary>
    /// Get the assembly handle
    /// </summary>
    public IntPtr Handle => _handle;

    /// <summary>
    /// Get the assembly path
    /// </summary>
    public string AssemblyPath => _assemblyPath;

    /// <summary>
    /// Get a function pointer to a specific method
    /// </summary>
    /// <typeparam name="T">Delegate type</typeparam>
    /// <param name="typeName">Fully qualified name of the type containing the method</param>
    /// <param name="methodName">Name of the method to get a pointer to</param>
    /// <returns>Function delegate</returns>
    public T GetFunction<T>(string typeName, string methodName) where T : Delegate
    {
        ThrowIfDisposed();

        var key = $"{_assemblyPath}:{typeName}.{methodName}";
        if (_cachedDelegates.TryGetValue(key, out var cachedDelegate))
        {
            return (T)cachedDelegate;
        }

        var function = _host.GetFunction<T>(_handle, typeName, methodName);
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

    protected virtual void Dispose(bool disposing)
    {
        if (!_isDisposed)
        {
            if (disposing)
            {
                _cachedDelegates.Clear();
                _host.UnloadAssembly(_handle);
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
            throw new ObjectDisposedException(nameof(Assembly));
        }
    }
}