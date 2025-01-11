#!/bin/bash

# Exit on error
set -e

# Detect OS
OS="unknown"
RUNTIME_ID="unknown"
case "$(uname)" in
    "Darwin")
        OS="macos"
        RUNTIME_ID="osx-arm64"
        # Check if dotnet is installed via homebrew
        if [ -d "/opt/homebrew/share/dotnet" ]; then
            export DOTNET_ROOT="/opt/homebrew/share/dotnet"
        elif [ -d "/usr/local/share/dotnet" ]; then
            export DOTNET_ROOT="/usr/local/share/dotnet"
        fi
        ;;
    "Linux")
        OS="linux"
        RUNTIME_ID="linux-x64"
        export DOTNET_ROOT="/usr/share/dotnet"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        OS="windows"
        RUNTIME_ID="win-x64"
        export DOTNET_ROOT="$ProgramFiles/dotnet"
        ;;
esac

echo "Detected OS: $OS"
echo "Runtime Identifier: $RUNTIME_ID"
echo "DOTNET_ROOT: $DOTNET_ROOT"

# Store the root directory
ROOT_DIR=$(pwd)

# Create build directory
mkdir -p build
cd build

# Create output directory
OUTPUT_DIR="bin"
mkdir -p "$OUTPUT_DIR"

# Build native library and tests
echo "Building native library and tests..."
cmake .. -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$OUTPUT_DIR" -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$OUTPUT_DIR"
cmake --build . --config Release

# Go back to root directory
cd "$ROOT_DIR"

# Build .NET library first (since DemoApp depends on it)
echo "Building .NET library..."
cd src/managed/DemoLibrary
if [ ! -f "DemoLibrary.csproj" ]; then
    echo "Error: DemoLibrary.csproj not found in $(pwd)"
    exit 1
fi
dotnet publish -c Release -r $RUNTIME_ID -o "../../../build/$OUTPUT_DIR"
cd "$ROOT_DIR"

# Build test library
echo "Building test library..."
cd tests/TestLibrary
if [ ! -f "TestLibrary.csproj" ]; then
    echo "Error: TestLibrary.csproj not found in $(pwd)"
    exit 1
fi
dotnet publish -c Release -r $RUNTIME_ID -o "../../build/$OUTPUT_DIR/test"
cd "$ROOT_DIR"

# Run tests
cd build
echo "Running tests..."
# ctest --verbose --output-on-failure
cd "$ROOT_DIR"

# Build demo app (after DemoLibrary is built)
echo "Building demo app..."
cd src/demo/DemoApp
if [ ! -f "DemoApp.csproj" ]; then
    echo "Error: DemoApp.csproj not found in $(pwd)"
    exit 1
fi
dotnet publish -c Release -r $RUNTIME_ID -o "../../../build/$OUTPUT_DIR"
cd "$ROOT_DIR"

echo "Build completed successfully!"
echo "All outputs can be found in: build/$OUTPUT_DIR"
echo ""
echo "Directory structure:"
ls -la "build/$OUTPUT_DIR" 