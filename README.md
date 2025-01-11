# 使用 Native Hosting 对 AOT 应用程序进行托管程序集扩展

本项目演示如何为 AOT 编译的 .NET 应用程序创建原生托管扩展。它包含一个原生托管库，可以加载和执行经过 AOT 编译的 .NET 程序集中的方法。

## 项目结构

- `src/native/` - 原生托管库（C++）
- `src/managed/DemoLibrary/` - 待加载的 .NET 库
- `src/demo/DemoApp/` - 演示应用程序
- `tests/` - 原生托管库的单元测试

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
cd src/managed/DemoLibrary
dotnet publish -c Release

cd ../demo/DemoApp
dotnet publish -c Release
```

## 运行演示

1. 将原生库从构建输出复制到演示应用的发布目录
2. 运行演示应用程序：

```bash
cd src/demo/DemoApp/bin/Release/net8.0/publish
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

## 使用示例

```csharp
// 在 .NET 库中定义方法
public static class Calculator
{
    public static int Add(int a, int b)
    {
        return a + b;
    }
}

// 在原生代码中调用
void* fnPtr = LoadAssemblyAndGetFunctionPointer(
    "DemoLibrary.dll",
    "DemoLibrary.Calculator",
    "Add",
    "DemoApp.AddDelegate");

// 转换为函数指针并调用
auto fn = reinterpret_cast<int(*)(int,int)>(fnPtr);
int result = fn(5, 3); // 结果为 8
```

## 开发说明

1. 原生库开发
   - 使用 CMake 进行跨平台构建
   - 实现了 .NET 主机 API 的包装
   - 提供了简单的错误处理机制

2. .NET 库开发
   - 支持 AOT 编译
   - 提供了清晰的公共 API
   - 包含完整的类型信息

3. 测试覆盖
   - 单元测试覆盖主要功能
   - 跨平台兼容性测试
   - 错误处理测试

## 许可证

MIT 