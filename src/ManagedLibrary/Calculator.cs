using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Reflection;
using Microsoft.Extensions.Logging;

namespace ManagedLibrary;

public class Calculator
{
    private static readonly ILogger<Calculator> Logger = ManagedLibrary2.LoggerFactory.CreateLogger<Calculator>();
    private static Assembly? _dynamicAssembly;
    private static Type? _dynamicCalculatorType;

    static Calculator()
    {

    }

    [UnmanagedCallersOnly]
    public static void Hello()
    {
        try
        {
            string assemblyLocation = Assembly.GetExecutingAssembly().Location;
            string assemblyDirectory = Path.GetDirectoryName(assemblyLocation);

            AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;


            string assemblyPath = Path.Combine(assemblyDirectory, "ManagedLibrary3.dll");
            _dynamicAssembly = Assembly.LoadFrom(assemblyPath);
            _dynamicCalculatorType = _dynamicAssembly.GetType("ManagedLibrary3.DynamicCalculator");
            Logger.LogInformation("Successfully loaded ManagedLibrary3");
        }
        catch (Exception ex)
        {
            Logger.LogError(ex, "Failed to load ManagedLibrary3");
        }
        Logger.LogInformation("Hello, World!");
    }

    private static Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
    {
        string dependencyPath = GetDependencyPath(args.Name);
        if (File.Exists(dependencyPath))
        {
            return Assembly.LoadFrom(dependencyPath);
        }
        return null;
    }

    // 获取依赖的 DLL 的路径
    private static string GetDependencyPath(string assemblyName)
    {
        string assemblyLocation = Assembly.GetExecutingAssembly().Location;
        string assemblyDirectory = Path.GetDirectoryName(assemblyLocation);
        string dependencyPath = Path.Combine(assemblyDirectory, assemblyName.Split(',')[0] + ".dll");
        return dependencyPath;
    }

    [UnmanagedCallersOnly]
    public static int Add(int a, int b)
    {
        try
        {
            if (_dynamicCalculatorType == null)
            {
                Logger.LogError("DynamicCalculator not loaded");
                return 0;
            }

            var method = _dynamicCalculatorType.GetMethod("Add") ?? throw new InvalidOperationException("Add method not found");
            var result = method.Invoke(null, new object[] { a, b });
            return result != null ? (int)result : 0;
        }
        catch (Exception ex)
        {
            Logger.LogError(ex, "Error calling dynamic Add");
            return 0;
        }
    }

    [UnmanagedCallersOnly]
    public static int Subtract(int a, int b)
    {
        try
        {
            if (_dynamicCalculatorType == null)
            {
                Logger.LogError("DynamicCalculator not loaded");
                return 0;
            }

            var method = _dynamicCalculatorType.GetMethod("Subtract") ?? throw new InvalidOperationException("Subtract method not found");
            var result = method.Invoke(null, new object[] { a, b });
            return result != null ? (int)result : 0;
        }
        catch (Exception ex)
        {
            Logger.LogError(ex, "Error calling dynamic Subtract");
            return 0;
        }
    }

    [UnmanagedCallersOnly]
    public static int Multiply(int a, int b)
    {
        try
        {
            if (_dynamicCalculatorType == null)
            {
                Logger.LogError("DynamicCalculator not loaded");
                return 0;
            }

            var method = _dynamicCalculatorType.GetMethod("Multiply") ?? throw new InvalidOperationException("Multiply method not found");
            var result = method.Invoke(null, new object[] { a, b });
            return result != null ? (int)result : 0;
        }
        catch (Exception ex)
        {
            Logger.LogError(ex, "Error calling dynamic Multiply");
            return 0;
        }
    }

    [UnmanagedCallersOnly]
    public static int Divide(int a, int b)
    {
        try
        {
            if (_dynamicCalculatorType == null)
            {
                Logger.LogError("DynamicCalculator not loaded");
                return 0;
            }

            var method = _dynamicCalculatorType.GetMethod("Divide") ?? throw new InvalidOperationException("Divide method not found");
            var result = method.Invoke(null, new object[] { a, b });
            return result != null ? (int)result : 0;
        }
        catch (Exception ex)
        {
            Logger.LogError(ex, "Error calling dynamic Divide");
            return 0;
        }
    }
}