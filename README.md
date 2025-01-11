# Native AOT Plugin Host

本项目演示如何为 AOT 编译的 .NET 应用程序创建原生插件扩展系统。它包含一个原生插件宿主库，可以在 AOT 编译的应用程序中动态加载和执行托管程序集中的方法。

## 项目结构

```
src/
├── native_aot_plugin_host/    # 原生插件宿主库（C++）
├── managed/
│   └── ManagedLibrary/       # 示例托管插件库
└── DemoApp/                  # 演示应用程序
tests/                        # 单元测试
```

## 环境要求

- CMake 3.15 或更高版本
- .NET SDK 8.0 或更高版本
- C++ 编译器
  - Windows: Visual Studio 2019+
  - Linux: GCC
  - macOS: Clang

## 构建项目

### 1. 构建原生库

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 2. 构建 .NET 项目

```bash
cd src/managed/ManagedLibrary
dotnet publish -c Release

cd ../../DemoApp
dotnet publish -c Release
```

## 运行演示

1. 将原生库从构建输出复制到演示应用的发布目录
2. 运行演示应用程序：

```bash
cd src/DemoApp/bin/Release/net8.0/publish
./DemoApp
```

## 运行测试

```bash
cd build
ctest --verbose
```

## 功能特点

- 跨平台支持（Windows、Linux、macOS）
- AOT 编译支持
- 动态加载 .NET 程序集
- 通过函数指针调用方法
- 错误处理和运行时管理
- 安全的资源管理（IDisposable 实现）

## 使用示例

```csharp
// 在托管库中定义方法
namespace ManagedLibrary;

public static class Calculator
{
    public static int Add(int a, int b)
    {
        return a + b;
    }
}

// 在 AOT 应用程序中使用
using var pluginHost = new NativeAotPluginHost();
pluginHost.Initialize("ManagedLibrary.runtimeconfig.json");

var add = pluginHost.GetFunction<AddDelegate>(
    "ManagedLibrary.dll",
    "ManagedLibrary.Calculator",
    "Add");

int result = add(5, 3);  // 结果: 8
```

## 许可证

MIT 