# Native Hosting Extension Demo

This project demonstrates how to create a native hosting extension for .NET applications with AOT support. It includes a native hosting library that can load and execute methods from AOT-compiled .NET assemblies.

## Project Structure

- `src/native/` - Native hosting library (C++)
- `src/managed/DemoLibrary/` - .NET library to be loaded
- `src/demo/DemoApp/` - Demo application showing usage
- `tests/` - Unit tests for the native hosting library

## Prerequisites

- CMake 3.15 or higher
- .NET SDK 8.0 or higher
- C++ compiler (Visual Studio 2019+ on Windows, GCC on Linux, Clang on macOS)

## Building the Project

### 1. Build the Native Library

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 2. Build the .NET Projects

```bash
cd src/managed/DemoLibrary
dotnet publish -c Release

cd ../demo/DemoApp
dotnet publish -c Release
```

## Running the Demo

1. Copy the native library from the build output to the demo app's publish directory
2. Run the demo application:

```bash
cd src/demo/DemoApp/bin/Release/net8.0/publish
./DemoApp
```

## Running Tests

```bash
cd build
ctest --verbose
```

## Features

- Cross-platform support (Windows, Linux, macOS)
- AOT compilation support
- Dynamic loading of .NET assemblies
- Method invocation through function pointers
- Error handling and runtime management

## License

MIT 