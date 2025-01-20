using System.Runtime.InteropServices;
using NativeHost;

namespace DemoApp;

class Program
{
    // Define delegates matching Calculator.cs methods
    private delegate int CalculateDelegate(int a, int b);
    private delegate void HelloDelegate();

    static void Main(string[] args)
    {
        // Create native host instance
        using var host = new NativeHost.NativeHost();
        Console.WriteLine("Native host initialized successfully.");

        try
        {
            // Get paths
            var baseDir = AppContext.BaseDirectory;
            var libraryPath = Path.Combine(baseDir, "ManagedLibrary.dll");

            // Load managed library
            Console.WriteLine($"\nLoading library: {libraryPath}");
            using var library = host.Load(libraryPath);
            Console.WriteLine($"Library loaded successfully. Handle: 0x{library.Handle:X}");

            // Get managed type information
            const string TypeName = "ManagedLibrary.Calculator";
            Console.WriteLine($"\nAccessing type: {TypeName}");

            // Get function pointers
            var hello = library.GetFunction<HelloDelegate>($"{TypeName}, ManagedLibrary", "Hello");
            var add = library.GetFunction<CalculateDelegate>($"{TypeName}, ManagedLibrary", "Add");
            var subtract = library.GetFunction<CalculateDelegate>($"{TypeName}, ManagedLibrary", "Subtract");

            // Test functions
            Console.WriteLine("\nTesting managed functions:");

            Console.WriteLine("Calling Hello():");
            hello();

            var a = 10;
            var b = 5;

            Console.WriteLine($"\nTesting calculator operations:");
            var sum = add(a, b);
            var difference = subtract(a, b);

            Console.WriteLine($"{a} + {b} = {sum}");
            Console.WriteLine($"{a} - {b} = {difference}");

        }
        catch (Exception ex)
        {
            Console.WriteLine($"\nError: {ex.Message}");
            if (ex.InnerException != null)
            {
                Console.WriteLine($"Inner Error: {ex.InnerException.Message}");
            }
        }

        Console.WriteLine("\nDemo completed.");
    }
}