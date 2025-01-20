using System.Runtime.InteropServices;
using Microsoft.Extensions.Logging;

namespace ManagedLibrary;

public class Calculator
{
    private static readonly ILogger<Calculator> Logger = ManagedLibrary2.LoggerFactory.CreateLogger<Calculator>();

    [UnmanagedCallersOnly]
    public static void Hello()
    {
        Logger.LogInformation("Hello, World!");
    }

    [UnmanagedCallersOnly]
    public static int Add(int a, int b)
    {
        var result = a + b;
        Logger.LogInformation("Add: {A} + {B} = {Result}", a, b, result);
        return result;
    }

    [UnmanagedCallersOnly]
    public static int Subtract(int a, int b)
    {
        var result = a - b;
        Logger.LogInformation("Subtract: {A} - {B} = {Result}", a, b, result);
        return result;
    }
}