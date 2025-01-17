using Microsoft.Extensions.Logging;

namespace ManagedLibrary2;

public class Class1
{
    private readonly ILogger<Class1> _logger;

    public Class1()
    {
        _logger = LoggerFactory.CreateLogger<Class1>();
    }

    public void DoSomething()
    {
        _logger.LogInformation("Hello from ManagedLibrary2!");
        _logger.LogDebug("This is a debug message");
        _logger.LogWarning("This is a warning message");
        _logger.LogError("This is an error message");

        try
        {
            throw new Exception("Test exception for logging");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Caught an exception");
        }
    }
}
