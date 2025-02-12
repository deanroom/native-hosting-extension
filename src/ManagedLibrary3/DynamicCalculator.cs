using Microsoft.Extensions.Logging;

namespace ManagedLibrary3;

public class DynamicCalculator
{
    private static readonly ILogger<DynamicCalculator> Logger = ManagedLibrary2.LoggerFactory.CreateLogger<DynamicCalculator>();

    public static int Add(int a, int b)
    {
        var result = a + b;
        Logger.LogInformation("Add: {A} + {B} = {Result}", a, b, result);
        return result;
    }

    public static int Subtract(int a, int b)
    {
        var result = a - b;
        Logger.LogInformation("Subtract: {A} - {B} = {Result}", a, b, result);
        return result;
    }

    public static int Multiply(int a, int b)
    {
        var result = a * b;
        Logger.LogInformation("Multiply: {A} * {B} = {Result}", a, b, result);
        return result;
    }

    public static int Divide(int a, int b)
    {
        if (b == 0)
        {
            Logger.LogError("Division by zero attempted");
            throw new DivideByZeroException();
        }
        var result = a / b;
        Logger.LogInformation("Divide: {A} / {B} = {Result}", a, b, result);
        return result;
    }
} 