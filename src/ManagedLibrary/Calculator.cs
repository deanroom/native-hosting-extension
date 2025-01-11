using System.Runtime.InteropServices;
using Microsoft.Extensions.Logging;

namespace ManagedLibrary;

public static class Calculator
{
    private static ILogger? _logger;
    
    [UnmanagedCallersOnly]
    public static void SetLogger(IntPtr logger)
    {
        _logger = (ILogger)GCHandle.FromIntPtr(logger).Target!;
    }

    [UnmanagedCallersOnly]
    public static void Hello()
    {
        _logger?.LogInformation("Hello method called");
        Console.WriteLine("Hello, World!");
    }

    [UnmanagedCallersOnly]
    public static int Add(int a, int b)
    {
        _logger?.LogInformation("Adding {A} and {B}", a, b);
        var result = a + b;
        _logger?.LogInformation("Add result: {Result}", result);
        return result;
    }

    [UnmanagedCallersOnly]
    public static int Subtract(int a, int b)
    {
        _logger?.LogInformation("Subtracting {B} from {A}", b, a);
        var result = a - b;
        _logger?.LogInformation("Subtract result: {Result}", result);
        return result;
    }

    public static int Multiply(int a, int b)
    {
        _logger?.LogInformation("Multiplying {A} and {B}", a, b);
        var result = a * b;
        _logger?.LogInformation("Multiply result: {Result}", result);
        return result;
    }

    public static int Divide(int a, int b)
    {
        _logger?.LogInformation("Dividing {A} by {B}", a, b);
        if (b == 0)
        {
            _logger?.LogError("Division by zero attempted");
            throw new DivideByZeroException();
        }
        var result = a / b;
        _logger?.LogInformation("Division result: {Result}", result);
        return result;
    }
}