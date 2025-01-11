using System.Runtime.InteropServices;
using Microsoft.Extensions.Logging;

namespace ManagedLibrary;

public static class Calculator
{

    [UnmanagedCallersOnly]
     public static void SetLogger(IntPtr logger)
    {
        var logger1 = (ILogger)GCHandle.FromIntPtr(logger).Target!;
        Console.WriteLine("Logger set");
        logger1.LogInformation("Logger set");
    }

    [UnmanagedCallersOnly]
    public static void Hello()
    {
        Console.WriteLine("Hello method called");
        Console.WriteLine("Hello, World!");
    }

    [UnmanagedCallersOnly]
    public static int Add(int a, int b)
    {
        Console.WriteLine($"Adding {a} and {b}");
        var result = a + b;
        Console.WriteLine($"Add result: {result}");
        return result;
    }

    [UnmanagedCallersOnly]
    public static int Subtract(int a, int b)
    {
        Console.WriteLine($"Subtracting {b} from {a}");
        var result = a - b;
        Console.WriteLine($"Subtract result: {result}");
        return result;
    }

    public static int Multiply(int a, int b)
    {
        Console.WriteLine($"Multiplying {a} and {b}");
        var result = a * b;
        Console.WriteLine($"Multiply result: {result}");
        return result;
    }

    public static int Divide(int a, int b)
    {
        Console.WriteLine($"Dividing {a} by {b}");
        if (b == 0)
        {
            Console.WriteLine("Error: Division by zero attempted");
            throw new DivideByZeroException();
        }
        var result = a / b;
        Console.WriteLine($"Division result: {result}");
        return result;
    }
}