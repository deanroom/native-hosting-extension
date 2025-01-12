using System.Runtime.InteropServices;
using NativePluginHost;

namespace DemoApp;

class Program
{
    // Consolidate delegates into a single location
    private delegate int CalculationDelegate(int a, int b);
    private delegate void VoidDelegate();

    static void Main(string[] args)
    {
        using var pluginHost = new PluginHost();
        
        try
        {
            var baseDir = AppContext.BaseDirectory;
            var runtimeConfigPath = Path.Combine(baseDir, "ManagedLibrary.runtimeconfig.json");
            var assemblyPath = Path.Combine(baseDir, "ManagedLibrary.dll");
            var typeName = "ManagedLibrary.Calculator, ManagedLibrary";

            pluginHost.Initialize(runtimeConfigPath);

            // Load and test functions
            var add = pluginHost.GetFunction<CalculationDelegate>(assemblyPath, typeName, "Add");
            var subtract = pluginHost.GetFunction<CalculationDelegate>(assemblyPath, typeName, "Subtract");
            var hello = pluginHost.GetFunction<VoidDelegate>(assemblyPath, typeName, "Hello");

            Console.WriteLine($"Add(5, 3) = {add(5, 3)}");
            Console.WriteLine($"Subtract(10, 4) = {subtract(10, 4)}");
            hello();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error: {ex.Message}");
        }
    }
}

