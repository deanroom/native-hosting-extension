using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using DemoApp;

public class CalculatorDemoService : BackgroundService
{
    private readonly ILogger<CalculatorDemoService> _logger;
    private readonly NativeAotPluginHost _pluginHost;
    private AddDelegate? _add;
    private SubtractDelegate? _subtract;
    private Hello? _hello;

    // Define delegate types
    private delegate int AddDelegate(int a, int b);
    private delegate int SubtractDelegate(int a, int b);
    private delegate void Hello();

    public CalculatorDemoService(ILogger<CalculatorDemoService> logger, NativeAotPluginHost pluginHost)
    {
        _logger = logger;
        _pluginHost = pluginHost;
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
            _logger.LogError(ex, "An error occurred while running the calculator demo");
        }
    }

    private void InitializePluginHost(CancellationToken stoppingToken)
    {
        string runtimeConfigPath = Path.Combine(AppContext.BaseDirectory, "ManagedLibrary.runtimeconfig.json");
        string assemblyPath = Path.Combine(AppContext.BaseDirectory, "ManagedLibrary.dll");

        _logger.LogInformation("Loading assembly from: {AssemblyPath}", assemblyPath);
        _logger.LogInformation("Using config from: {ConfigPath}", runtimeConfigPath);

        _logger.LogInformation("Initializing runtime...");
        _pluginHost.Initialize(runtimeConfigPath);

        // Load Add method
        _logger.LogInformation("Loading Add method...");
        _add = _pluginHost.GetFunction<AddDelegate>(
            assemblyPath,
            "ManagedLibrary.Calculator, ManagedLibrary",
            "Add");

        // Load Subtract method
        _logger.LogInformation("Loading Subtract method...");
        _subtract = _pluginHost.GetFunction<SubtractDelegate>(
            assemblyPath,
            "ManagedLibrary.Calculator, ManagedLibrary",
            "Subtract");

        // Load Hello method
        _logger.LogInformation("Loading Hello method...");
        _hello = _pluginHost.GetFunction<Hello>(
            assemblyPath,
            "ManagedLibrary.Calculator, ManagedLibrary",
            "Hello");

        _logger.LogInformation("Calculator is ready. Available commands:");
        _logger.LogInformation("- add(x,y)");
        _logger.LogInformation("- subtract(x,y)");
        _logger.LogInformation("- hello");
        _logger.LogInformation("Press Ctrl+C to exit");
    }

    private async Task ProcessUserCommands(CancellationToken stoppingToken)
    {
        while (!stoppingToken.IsCancellationRequested)
        {
            if (Console.KeyAvailable)
            {
                await Console.Out.WriteLineAsync("Enter command :");
                var input = await Console.In.ReadLineAsync(stoppingToken);
                if (string.IsNullOrEmpty(input)) continue;

                var command = CommandParser.ParseCommand(input);
                if (command == null)
                {
                    _logger.LogWarning("Invalid command format. Available commands:");
                    _logger.LogWarning("- add(x,y)");
                    _logger.LogWarning("- subtract(x,y)");
                    _logger.LogWarning("- hello");
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
                            _logger.LogInformation("Result: {Result}", addResult);
                            break;

                        case "subtract" when a.HasValue && b.HasValue:
                            int subtractResult = _subtract?.Invoke(a.Value, b.Value) ??
                                throw new InvalidOperationException("Subtract function not loaded");
                            _logger.LogInformation("Result: {Result}", subtractResult);
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
                    _logger.LogError(ex, "Error executing command");
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
        _logger.LogInformation("Stopping calculator demo service");
        _pluginHost.Dispose();
        await base.StopAsync(cancellationToken);
    }
}