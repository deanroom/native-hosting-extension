# Native Plugin Host

本项目演示如何为 AOT 编译的 .NET 应用程序创建基于托管程序集的插件扩展。它包含一个原生插件宿主库，可以在 AOT 编译的应用程序中动态加载和执行托管程序集中的方法。

## 项目结构

```
src/
├── native_plugin_host/   # 原生插件宿主库（C++）
├── NativePluginHost/     # .NET 插件宿主包装库
├── ManagedLibrary/      # 示例托管插件库
└── DemoApp/             # 演示应用程序
```

## 环境要求

- CMake 3.15 或更高版本
- .NET SDK 8.0 或更高版本
- C++ 编译器
  - Windows: Visual Studio 2019+
  - Linux: GCC
  - macOS: Clang

## 构建项目

```bash
# 克隆仓库
git clone https://github.com/yourusername/native-plugin-host.git
cd native-plugin-host

# 构建项目
./build.sh  # Linux/macOS
./build.ps1   # Windows
```

## 使用示例

```csharp
// 创建插件宿主
using var pluginHost = new PluginHost();

try
{
    var baseDir = AppContext.BaseDirectory;
    
    // 加载计算器插件
    var calculatorConfigPath = Path.Combine(baseDir, "ManagedLibrary.runtimeconfig.json");
    var calculatorAssemblyPath = Path.Combine(baseDir, "ManagedLibrary.dll");
    var calculatorTypeName = "ManagedLibrary.Calculator, ManagedLibrary";
    
    using var calculator = pluginHost.LoadPlugin(calculatorConfigPath);
    
    // 获取计算器函数
    var add = calculator.GetFunction<Func<int, int, int>>(
        calculatorAssemblyPath,
        calculatorTypeName,
        "Add"
    );
    
    // 使用插件
    int result = add(5, 3);
    Console.WriteLine($"Result: {result}");
}
catch (Exception ex)
{
    Console.WriteLine($"Error: {ex.Message}");
}
```

## 开发插件

创建新的 .NET 类库项目：

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net8.0</TargetFramework>
    <GenerateRuntimeConfigurationFiles>true</GenerateRuntimeConfigurationFiles>
    <EnableDynamicLoading>true</EnableDynamicLoading>
  </PropertyGroup>
</Project>
```

- 项目配置说明：

  - `TargetFramework`: 指定目标框架版本。
  - `GenerateRuntimeConfigurationFiles`: 生成 `.runtimeconfig.json` 文件，该文件包含运行时配置信息，是插件加载所必需的。
  - `EnableDynamicLoading`: 启用动态加载支持，允许程序集在运行时被动态加载。这会将程序集编译为可重定位的形式，便于插件系统加载。

- 实现插件类：

```csharp
public class Calculator
{
    [UnmanagedCallersOnly]
    public static int Add(int a, int b)
    {
        return a + b;
    }
}

```

## 功能特点

- 跨平台支持（Windows、Linux、macOS）
- 支持多插件并行加载和执行
- 自动委托缓存机制
- 完整的资源生命周期管理
- 详细的错误处理机制

## 限制说明

- 只支持 blittable 类型的参数
- 方法必须是静态的
- 不支持泛型参数
- 不支持引用参数
- 需要 .NET 8.0 或更高版本
- 需要支持 AOT 编译

## 更多文档

详细的设计文档和 API 参考，请查看 [doc/native-plugin-host.md](doc/native-plugin-host.md)。