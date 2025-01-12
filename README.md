# Native Plugin Host

本项目演示如何为 AOT 编译的 .NET 应用程序创建基于托管程序集的插件扩展。它包含一个原生插件宿主库，可以在 AOT 编译的应用程序中动态加载和执行托管程序集中的方法。

## 项目结构

```
src/
├── native_plugin_host/   # 原生插件宿主库（C++）
│── NativePluginHost/      # .NET 插件宿主包装库(在编译为 AOT 的 .NET应用程序中使用)
│── ManagedLibrary/           # 示例托管插件库
└── DemoApp/                  # 演示应用程序（AOT 编译）
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

```bash
./build.sh
```

## 运行演示

1. 将原生库从构建输出复制到演示应用的发布目录
2. 运行演示应用程序：

```bash
cd build/bin
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
- 独立的 .NET 插件宿主包装库

## 使用示例

```csharp
// 在托管插件库中定义方法
namespace ManagedLibrary;

public static class Calculator
{
    public static int Add(int a, int b)
    {
        return a + b;
    }
}

// 在 AOT 应用程序中使用
using NativePluginHost;

using var pluginHost = new NativePluginHost();
pluginHost.Initialize("ManagedLibrary.runtimeconfig.json");

var add = pluginHost.GetFunction<AddDelegate>(
    "ManagedLibrary.dll",
    "ManagedLibrary.Calculator",
    "Add");

int result = add(5, 3);  // 结果: 8
```

## 组件说明

1. **native_plugin_host**
   - C++ 实现的原生插件宿主库
   - 提供底层的程序集加载和函数调用功能
   - 跨平台实现（Windows/Linux/macOS）

2. **NativePluginHost (.NET)**
   - 原生库的 .NET 包装器
   - 提供友好的 .NET API
   - 实现 IDisposable 接口确保资源正确释放

3. **ManagedLibrary**
   - 示例托管插件库
   - 展示如何编写可被动态加载的插件

4. **DemoApp**
   - AOT 编译的演示应用
   - 展示如何在 AOT 环境中使用插件系统