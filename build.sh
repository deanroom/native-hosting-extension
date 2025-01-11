#!/bin/bash

# Exit on error
set -e

# Detect OS
OS="unknown"
RUNTIME_ID="unknown"
case "$(uname)" in
    "Darwin")
        OS="macos"
        RUNTIME_ID="osx-x64"
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

# Build test library first
echo "Building test library..."
cd tests/TestLibrary
dotnet publish -c Release -r $RUNTIME_ID
cd ../..

# Create build directory for native code
echo "Building native library and tests..."
mkdir -p build
cd build
cmake ..
cmake --build . --config Release

# Run tests
echo "Running tests..."
ctest --verbose --output-on-failure
cd ..

# Build .NET library
echo "Building .NET library..."
cd src/managed/DemoLibrary
dotnet publish -c Release -r $RUNTIME_ID
cd ../../..

# Build demo app
echo "Building demo app..."
cd src/demo/DemoApp
dotnet publish -c Release -r $RUNTIME_ID
cd ../../..

# Copy native library to demo app directory
echo "Copying native library to demo app..."
DEMO_APP_DIR="src/demo/DemoApp/bin/Release/net8.0/$RUNTIME_ID/publish"
mkdir -p "$DEMO_APP_DIR"

if [ "$OS" = "windows" ]; then
    cp build/bin/Release/NativeHosting.dll "$DEMO_APP_DIR/"
elif [ "$OS" = "macos" ]; then
    cp build/lib/libNativeHosting.dylib "$DEMO_APP_DIR/"
else
    cp build/lib/libNativeHosting.so "$DEMO_APP_DIR/"
fi

# Copy test artifacts to test directory
TEST_DIR="build/tests"
mkdir -p "$TEST_DIR"
cp "tests/TestLibrary/bin/Release/net8.0/$RUNTIME_ID/publish/TestLibrary.dll" "$TEST_DIR/"
cp "tests/TestLibrary/bin/Release/net8.0/$RUNTIME_ID/publish/TestLibrary.runtimeconfig.json" "$TEST_DIR/"

echo "Build completed successfully!"
echo "You can find the demo app in: $DEMO_APP_DIR" 