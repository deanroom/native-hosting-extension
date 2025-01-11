using System.Runtime.InteropServices;

namespace DemoLibrary;

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
        return a + b;
    }

    [UnmanagedCallersOnly]
    public static int Subtract(int a, int b)
    {
        return a - b;
    }

    public static int Multiply(int a, int b)
    {
        return a * b;
    }

    public static int Divide(int a, int b)
    {
        if (b == 0)
            throw new DivideByZeroException();
        return a / b;
    }
} 