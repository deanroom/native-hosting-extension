namespace TestLibrary;

public static class TestClass
{
    public static int ReturnConstant()
    {
        return 42;
    }

    public static string ReturnString()
    {
        return "Hello from managed code!";
    }

    public static int AddNumbers(int a, int b)
    {
        return a + b;
    }

    public static bool ThrowException()
    {
        throw new System.Exception("Test exception");
    }
} 