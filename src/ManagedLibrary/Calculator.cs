using System.Runtime.InteropServices;

namespace ManagedLibrary;

public static class Calculator
{
    [UnmanagedCallersOnly]
    public static void Hello()
    {
        Console.WriteLine("Hello, World!");
    }

    [UnmanagedCallersOnly]
    public static int Add(int a, int b)
    {
        var result = a + b;
        Console.WriteLine($"Add: {a} + {b} = {result}");
        return result;
    }

    [UnmanagedCallersOnly]
    public static int Subtract(int a, int b)
    {
        var result = a - b;
        Console.WriteLine($"Subtract: {a} - {b} = {result}");
        return result;
    }
}