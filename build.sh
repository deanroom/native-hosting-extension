#!/bin/bash

# Exit on error
set -e

# Detect OS
OS="unknown"
case "$(uname)" in
    "Darwin")
        OS="macos"
        ;;
    "Linux")
        OS="linux"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        OS="windows"
        ;;
esac

echo "Detected OS: $OS"

# Create build directory for native code
echo "Building native library..."
mkdir -p build
cd build
cmake ..
cmake --build . --config Release
cd ..

# Build .NET library
echo "Building .NET library..."
cd src/managed/DemoLibrary
dotnet publish -c Release
cd ../../..

# Build demo app
echo "Building demo app..."
cd src/demo/DemoApp
dotnet publish -c Release
cd ../../..

# Copy native library to demo app directory
echo "Copying native library to demo app..."
DEMO_APP_DIR="src/demo/DemoApp/bin/Release/net8.0/publish"
mkdir -p "$DEMO_APP_DIR"

if [ "$OS" = "windows" ]; then
    cp build/bin/Release/NativeHosting.dll "$DEMO_APP_DIR/"
elif [ "$OS" = "macos" ]; then
    cp build/lib/libNativeHosting.dylib "$DEMO_APP_DIR/"
else
    cp build/lib/libNativeHosting.so "$DEMO_APP_DIR/"
fi

echo "Build completed successfully!"
echo "You can find the demo app in: $DEMO_APP_DIR" 