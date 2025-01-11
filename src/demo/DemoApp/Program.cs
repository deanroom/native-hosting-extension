using System.Runtime.InteropServices;

partial class Program
{
    private const string LibraryName = "NativeHosting";

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static partial bool InitializeRuntime(string runtimeConfigPath);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    internal static partial IntPtr LoadAssemblyAndGetFunctionPointer(
        string assemblyPath,
        string typeName,
        string methodName,
        string delegateTypeName);

    [LibraryImport(LibraryName)]
    internal static partial void CloseRuntime();

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

            Console.WriteLine($"Loading assembly from: {assemblyPath}");
            Console.WriteLine($"Using config from: {runtimeConfigPath}");

            Console.WriteLine("Initializing runtime...");
            if (!InitializeRuntime(runtimeConfigPath))
            {
                Console.WriteLine("Failed to initialize runtime");
                return;
            }

            // Load and test Add method
            Console.WriteLine("Loading Add method...");
            var addPtr = LoadAssemblyAndGetFunctionPointer(
                assemblyPath,
                "DemoLibrary.Calculator",
                "Add",
                typeof(AddDelegate).ToString());

            if (addPtr == IntPtr.Zero)
            {
                Console.WriteLine("Failed to load Add method");
                return;
            }

            var add = Marshal.GetDelegateForFunctionPointer<AddDelegate>(addPtr);

            // Test the Add method
            int a = 5, b = 3;
            Console.WriteLine($"Calling Add({a}, {b})...");
            int result = add(a, b);
            Console.WriteLine($"Result: {a} + {b} = {result}");

            // Load and test Subtract method
            Console.WriteLine("\nLoading Subtract method...");
            var subtractPtr = LoadAssemblyAndGetFunctionPointer(
                assemblyPath,
                "DemoLibrary.Calculator",
                "Subtract",
                typeof(SubtractDelegate).ToString());

            if (subtractPtr == IntPtr.Zero)
            {
                Console.WriteLine("Failed to load Subtract method");
                return;
            }

            var subtract = Marshal.GetDelegateForFunctionPointer<SubtractDelegate>(subtractPtr);
            Console.WriteLine($"Calling Subtract({a}, {b})...");
            result = subtract(a, b);
            Console.WriteLine($"Result: {a} - {b} = {result}");

            Console.WriteLine("\nPress any key to exit...");
            Console.ReadKey();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error: {ex.Message}");
            Console.WriteLine(ex.StackTrace);
        }
        finally
        {
            CloseRuntime();
        }
    }
} 