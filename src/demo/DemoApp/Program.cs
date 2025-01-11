using System.Runtime.InteropServices;
using DemoLibrary;

class Program
{
    [DllImport("NativeHosting")]
    private static extern bool InitializeRuntime(string runtimeConfigPath);

    [DllImport("NativeHosting")]
    private static extern IntPtr LoadAssemblyAndGetFunctionPointer(
        string assemblyPath,
        string typeName,
        string methodName,
        string delegateTypeName);

    [DllImport("NativeHosting")]
    private static extern void CloseRuntime();

    // Define delegate types matching our Calculator methods
    private delegate int AddDelegate(int a, int b);
    private delegate int SubtractDelegate(int a, int b);
    private delegate int MultiplyDelegate(int a, int b);
    private delegate int DivideDelegate(int a, int b);

    static void Main(string[] args)
    {
        try
        {
            string runtimeConfigPath = Path.Combine(AppContext.BaseDirectory, "DemoLibrary.runtimeconfig.json");
            string assemblyPath = Path.Combine(AppContext.BaseDirectory, "DemoLibrary.dll");

            Console.WriteLine("Initializing runtime...");
            if (!InitializeRuntime(runtimeConfigPath))
            {
                Console.WriteLine("Failed to initialize runtime");
                return;
            }

            // Load Add method
            var addPtr = LoadAssemblyAndGetFunctionPointer(
                assemblyPath,
                "DemoLibrary.Calculator",
                "Add",
                "DemoApp.AddDelegate");

            var add = Marshal.GetDelegateForFunctionPointer<AddDelegate>(addPtr);

            // Test the Add method
            int result = add(5, 3);
            Console.WriteLine($"5 + 3 = {result}");

            // Similarly, you can load and test other methods...

            Console.WriteLine("Press any key to exit...");
            Console.ReadKey();
        }
        finally
        {
            CloseRuntime();
        }
    }
} 