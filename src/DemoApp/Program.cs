using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
namespace DemoApp;
using NativeAotPluginHost;

partial class Program
{
    static async Task Main(string[] args)
    {
        var builder = Host.CreateApplicationBuilder(args);

        // Add services to the container
        builder.Services.AddLogging(builder => builder.AddConsole());
        builder.Services.AddSingleton<PluginHost>();
        builder.Services.AddHostedService<CalculatorService>();

        // Build and run the host
        using var host = builder.Build();
        await host.RunAsync();
    }
}

public class CommandParser
{
    private static readonly Regex CommandRegex = new(@"^(add|subtract)\((\d+),(\d+)\)$|^(hello)$", RegexOptions.IgnoreCase);

    public static (string Command, int? A, int? B)? ParseCommand(string input)
    {
        var match = CommandRegex.Match(input.Trim());
        if (!match.Success) return null;

        // Check if it's the hello command
        if (match.Groups[4].Success)
        {
            return ("hello", null, null);
        }

        // Otherwise it's a calculator command
        var command = match.Groups[1].Value.ToLowerInvariant();
        if (int.TryParse(match.Groups[2].Value, out int a) &&
            int.TryParse(match.Groups[3].Value, out int b))
        {
            return (command, a, b);
        }

        return null;
    }
}

