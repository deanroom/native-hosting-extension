using System.Runtime.InteropServices;
using NativeHost;

namespace DemoApp;

class Program
{
    // Delegate definitions
    private delegate int CalculationDelegate(int a, int b);
    private delegate void VoidDelegate();

    static void Main(string[] args)
    {
        using var host = new NativeHost.NativeHost();

        try
        {
            var baseDir = AppContext.BaseDirectory;

            // Load the calculator assembly
            var calculatorAssemblyPath = Path.Combine(baseDir, "ManagedLibrary.dll");
            var calculatorTypeName = "ManagedLibrary.Calculator, ManagedLibrary";

            using var calculator = host.LoadAssembly(calculatorAssemblyPath);
            Console.WriteLine($"Loaded calculator assembly: {calculator.Handle}");

            // Get and test calculator functions
            var add = calculator.GetFunction<CalculationDelegate>(
                calculatorTypeName,
                "Add"
            );
            var subtract = calculator.GetFunction<CalculationDelegate>(
                calculatorTypeName,
                "Subtract"
            );
            var hello = calculator.GetFunction<VoidDelegate>(
                calculatorTypeName,
                "Hello"
            );

            // Test calculator functionality
            Console.WriteLine("Testing calculator assembly:");
            hello();
            Console.WriteLine($"Add(5, 3) = {add(5, 3)}");
            Console.WriteLine($"Subtract(10, 4) = {subtract(10, 4)}");

            // Demonstrate unloading assembly
            Console.WriteLine("\nUnloading calculator assembly...");
            calculator.Dispose();
            Console.WriteLine("Calculator assembly unloaded successfully.");

            // You can load more assemblies here...
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error: {ex.Message}");
            if (ex.InnerException != null)
            {
                Console.WriteLine($"Inner Error: {ex.InnerException.Message}");
            }
        }
    }
}