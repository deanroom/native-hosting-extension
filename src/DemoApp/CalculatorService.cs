using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using NativeAotPluginHost;
using Microsoft.Extensions.Logging;
using System.Runtime.InteropServices;
namespace DemoApp;

public class CalculatorService : BackgroundService
{
    private readonly PluginHost _pluginHost;
    private readonly ILogger<CalculatorService> _logger;
    private AddDelegate? _add;
    private SubtractDelegate? _subtract;
    private Hello? _hello;
    private SetLogger? _setLogger;
    private GCHandle _loggerHandle;


    // Define delegate types
    private delegate int AddDelegate(int a, int b);
    private delegate int SubtractDelegate(int a, int b);
    private delegate void Hello();
    private delegate void SetLogger(IntPtr logger);
    public CalculatorService(PluginHost pluginHost, ILogger<CalculatorService> logger)
    {
        _pluginHost = pluginHost;
        _logger = logger;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        try
        {
            InitializePluginHost(stoppingToken);
            await ProcessUserCommands(stoppingToken);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"An error occurred while running the calculator demo: {ex.Message}");
        }
    }

    private void InitializePluginHost(CancellationToken stoppingToken)
    {
        string runtimeConfigPath = Path.Combine(AppContext.BaseDirectory, "ManagedLibrary.runtimeconfig.json");
        string assemblyPath = Path.Combine(AppContext.BaseDirectory, "ManagedLibrary.dll");

        Console.WriteLine($"Loading assembly from: {assemblyPath}");
        Console.WriteLine($"Using config from: {runtimeConfigPath}");

        Console.WriteLine("Initializing runtime...");
        _pluginHost.Initialize(runtimeConfigPath);

        // Load Add method
        Console.WriteLine("Loading Add method...");
        _add = _pluginHost.GetFunction<AddDelegate>(
            assemblyPath,
            "ManagedLibrary.Calculator, ManagedLibrary",
            "Add");

        // Load Subtract method
        Console.WriteLine("Loading Subtract method...");
        _subtract = _pluginHost.GetFunction<SubtractDelegate>(
            assemblyPath,
            "ManagedLibrary.Calculator, ManagedLibrary",
            "Subtract");

        // Load Hello method
        Console.WriteLine("Loading Hello method...");
        _hello = _pluginHost.GetFunction<Hello>(
            assemblyPath,
            "ManagedLibrary.Calculator, ManagedLibrary",
            "Hello");

        // Load SetLogger method
        Console.WriteLine("Loading SetLogger method...");
        _setLogger = _pluginHost.GetFunction<SetLogger>(
            assemblyPath,
            "ManagedLibrary.Calculator, ManagedLibrary",
            "SetLogger");
        _loggerHandle = GCHandle.Alloc(_logger, GCHandleType.Normal);
        _setLogger?.Invoke(GCHandle.ToIntPtr(_loggerHandle));

        Console.WriteLine("Calculator is ready. Available commands:");
        Console.WriteLine("- add(x,y)");
        Console.WriteLine("- sub(x,y)");
        Console.WriteLine("- hello");
        Console.WriteLine("Press Ctrl+C to exit");
    }

    private async Task ProcessUserCommands(CancellationToken stoppingToken)
    {
        while (!stoppingToken.IsCancellationRequested)
        {
            if (Console.KeyAvailable)
            {
                var input = await Console.In.ReadLineAsync(stoppingToken);
                if (string.IsNullOrEmpty(input)) continue;

                var command = CommandParser.ParseCommand(input);
                if (command == null)
                {
                    Console.WriteLine("Invalid command format. Available commands:");
                    Console.WriteLine("- add(x,y)");
                    Console.WriteLine("- subtract(x,y)");
                    Console.WriteLine("- hello");
                    continue;
                }

                try
                {
                    var (operation, a, b) = command.Value;
                    switch (operation)
                    {
                        case "add" when a.HasValue && b.HasValue:
                            int addResult = _add?.Invoke(a.Value, b.Value) ??
                                throw new InvalidOperationException("Add function not loaded");
                            Console.WriteLine($"Result: {addResult}");
                            break;

                        case "sub" when a.HasValue && b.HasValue:
                            int subtractResult = _subtract?.Invoke(a.Value, b.Value) ??
                                throw new InvalidOperationException("Subtract function not loaded");
                            Console.WriteLine($"Result: {subtractResult}");
                            break;

                        case "hello":
                            if (_hello == null)
                                throw new InvalidOperationException("Hello function not loaded");
                            _hello.Invoke();
                            break;

                        default:
                            throw new InvalidOperationException($"Unknown operation: {operation}");
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error executing command: {ex.Message}");
                }
            }
            else
            {
                await Task.Delay(100, stoppingToken);
            }
        }
    }

    public override async Task StopAsync(CancellationToken cancellationToken)
    {
        Console.WriteLine("Stopping calculator demo service");
        _pluginHost.Dispose();
        await base.StopAsync(cancellationToken);
    }
}