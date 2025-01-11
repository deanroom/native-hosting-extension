using System.Runtime.InteropServices;

namespace TestLibrary;

public static class TestClass
{
    [UnmanagedCallersOnly]
    public static int ReturnConstant()
    {
        return 42;
    }

    [UnmanagedCallersOnly]
    public static int AddNumbers(int a, int b)
    {
        return a + b;
    }

    [UnmanagedCallersOnly]
    public static bool ThrowException()
    {
        throw new System.Exception("Test exception");
    }
}