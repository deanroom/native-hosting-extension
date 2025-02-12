
namespace ManagedLibrary3;

public class DynamicCalculator
{

    public static int Add(int a, int b)
    {
        var result = a + b;
        return result;
    }

    public static int Subtract(int a, int b)
    {
        var result = a - b;
        return result;
    }

    public static int Multiply(int a, int b)
    {
        var result = a * b;
        return result;
    }

    public static int Divide(int a, int b)
    {
        if (b == 0)
        {
            throw new DivideByZeroException();
        }
        var result = a / b;
        return result;
    }
}