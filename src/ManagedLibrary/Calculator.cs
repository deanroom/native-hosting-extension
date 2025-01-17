using System.Runtime.InteropServices;
using Microsoft.Extensions.Logging;
using ManagedLibrary2;

namespace ManagedLibrary;

public class Calculator
{
    private static readonly ILogger<Calculator> _logger = ManagedLibrary2.LoggerFactory.CreateLogger<Calculator>();

    [UnmanagedCallersOnly]
    public static void Hello()
    {
        _logger.LogInformation("Hello, World!");
    }

    [UnmanagedCallersOnly]
    public static int Add(int a, int b)
    {
        var result = a + b;
        _logger.LogInformation("Add: {A} + {B} = {Result}", a, b, result);
        return result;
    }

    [UnmanagedCallersOnly]
    public static int Subtract(int a, int b)
    {
        var result = a - b;
        _logger.LogInformation("Subtract: {A} - {B} = {Result}", a, b, result);
        return result;
    }
}