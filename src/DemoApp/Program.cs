using System.Runtime.InteropServices;
using NativePluginHost;

namespace DemoApp;

class Program
{
    // 委托定义
    private delegate int CalculationDelegate(int a, int b);
    private delegate void VoidDelegate();

    static void Main(string[] args)
    {
        using var pluginHost = new PluginHost();
        
        try
        {
            var baseDir = AppContext.BaseDirectory;
            
            // 加载第一个插件（计算器）
            var calculatorConfigPath = Path.Combine(baseDir, "ManagedLibrary.runtimeconfig.json");
            var calculatorAssemblyPath = Path.Combine(baseDir, "ManagedLibrary.dll");
            var calculatorTypeName = "ManagedLibrary.Calculator, ManagedLibrary";
            
            using var calculator = pluginHost.LoadPlugin(calculatorConfigPath);
            Console.WriteLine($"Loaded calculator plugin: {calculator.Handle}");

            // 获取并测试计算器函数
            var add = calculator.GetFunction<CalculationDelegate>(
                calculatorAssemblyPath, 
                calculatorTypeName, 
                "Add"
            );
            var subtract = calculator.GetFunction<CalculationDelegate>(
                calculatorAssemblyPath, 
                calculatorTypeName, 
                "Subtract"
            );
            var hello = calculator.GetFunction<VoidDelegate>(
                calculatorAssemblyPath, 
                calculatorTypeName, 
                "Hello"
            );

            // 测试计算器功能
            Console.WriteLine("Testing calculator plugin:");
            Console.WriteLine($"Add(5, 3) = {add(5, 3)}");
            Console.WriteLine($"Subtract(10, 4) = {subtract(10, 4)}");
            hello();

            // 演示卸载插件
            Console.WriteLine("\nUnloading calculator plugin...");
            calculator.Dispose();
            Console.WriteLine("Calculator plugin unloaded successfully.");

            // 可以在这里加载更多插件...
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error: {ex.Message}");
            if (ex.InnerException != null)
            {
                Console.WriteLine($"Inner Error: {ex.InnerException.Message}");
            }
        }
    }
}