using System.Runtime.InteropServices;
using NativeAotPluginHost;

namespace DemoApp;

class Program
{
    private delegate int AddDelegate(int a, int b);
    private delegate int SubtractDelegate(int a, int b);
    private delegate void Hello();

    static void Main(string[] args)
    {
        using var pluginHost = new PluginHost();
        
        try
        {
            // Initialize runtime
            string runtimeConfigPath = Path.Combine(AppContext.BaseDirectory, "ManagedLibrary.runtimeconfig.json");
            string assemblyPath = Path.Combine(AppContext.BaseDirectory, "ManagedLibrary.dll");

            Console.WriteLine($"Loading assembly from: {assemblyPath}");
            Console.WriteLine($"Using config from: {runtimeConfigPath}");
            Console.WriteLine("Initializing runtime...");
            
            pluginHost.Initialize(runtimeConfigPath);

            // Load functions
            var add = pluginHost.GetFunction<AddDelegate>(
                assemblyPath,
                "ManagedLibrary.Calculator, ManagedLibrary",
                "Add");

            var subtract = pluginHost.GetFunction<SubtractDelegate>(
                assemblyPath,
                "ManagedLibrary.Calculator, ManagedLibrary",
                "Subtract");

            var hello = pluginHost.GetFunction<Hello>(
                assemblyPath,
                "ManagedLibrary.Calculator, ManagedLibrary",
                "Hello");

            // Test the functions
            Console.WriteLine("\nTesting functions:");
            
            Console.WriteLine("Calling Add(5, 3)...");
            int addResult = add(5, 3);
            Console.WriteLine($"Result: {addResult}");

            Console.WriteLine("\nCalling Subtract(10, 4)...");
            int subtractResult = subtract(10, 4);
            Console.WriteLine($"Result: {subtractResult}");

            Console.WriteLine("\nCalling Hello()...");
            hello();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"An error occurred: {ex.Message}");
        }
    }
}

