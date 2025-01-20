using Microsoft.Extensions.Logging;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging.Console;
using Microsoft.Extensions.Logging.Abstractions;

namespace ManagedLibrary2;

public static class LoggerFactory
{
    private static ILoggerFactory? _factory;
    private static readonly object Lock = new();

    public static ILogger<T> CreateLogger<T>()
    {
        if (_factory == null)
        {
            lock (Lock)
            {
                if (_factory == null)
                {
                    var services = new ServiceCollection();
                    services.AddLogging(builder =>
                    {
                        builder.SetMinimumLevel(LogLevel.Debug)
                               .AddConsole()
                               .AddConsoleFormatter<SimpleConsoleFormatter, SimpleConsoleFormatterOptions>(options =>
                               {
                                   options.TimestampFormat = "HH:mm:ss ";
                               });
                    });

                    Console.WriteLine("LoggerFactory created");

                    var serviceProvider = services.BuildServiceProvider();
                    _factory = serviceProvider.GetRequiredService<ILoggerFactory>();
                }
            }
        }

        return _factory.CreateLogger<T>();
    }
}

public class SimpleConsoleFormatter : ConsoleFormatter
{
    public SimpleConsoleFormatter() : base("SimpleConsole") { }

    public override void Write<TState>(
        in LogEntry<TState> logEntry,
        IExternalScopeProvider? scopeProvider,
        TextWriter textWriter)
    {
        string? message = logEntry.Formatter?.Invoke(logEntry.State, logEntry.Exception);
        if (message == null)
        {
            return;
        }

        var logLevel = logEntry.LogLevel.ToString().ToUpper();
        textWriter.WriteLine($"[{DateTime.Now:HH:mm:ss}] [{logLevel}] {message}");
        
        if (logEntry.Exception != null)
        {
            textWriter.WriteLine(logEntry.Exception.ToString());
        }
    }
}

public class SimpleConsoleFormatterOptions : ConsoleFormatterOptions
{
} 