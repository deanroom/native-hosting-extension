#!/bin/bash
set -e

# Default values
BUILD_DIR="build"
CLEAN=1
RUN_TESTS=1
RUN_DEMO=1  # Default to not running the demo app
BUILD_TYPE="Debug"  # Default build type

# Set default compilers
if [ "$(uname)" = "Darwin" ]; then
    export CC=$(which clang)
    export CXX=$(which clang++)
elif [ "$(uname)" = "Linux" ]; then
    export CC=$(which gcc)
    export CXX=$(which g++)
fi

# Detect number of CPU cores
if [ "$(uname)" = "Darwin" ]; then
    NUM_CORES=$(sysctl -n hw.ncpu)
elif [ "$(uname)" = "Linux" ]; then
    NUM_CORES=$(nproc)
else
    NUM_CORES=4  # Default to 4 cores on other platforms
fi

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        --test)
            RUN_TESTS=1
            shift
            ;;
        --demo)
            RUN_DEMO=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--debug|--release] [--clean] [--test] [--demo]"
            exit 1
            ;;
    esac
done

# Clean build directory if requested
if [ $CLEAN -eq 1 ]; then
    echo "Cleaning build directory..."
    rm -rf $BUILD_DIR
fi

# Create build directory
mkdir -p $BUILD_DIR

echo "=== Building $BUILD_TYPE configuration ==="

# Configure CMake
echo "Configuring CMake for $BUILD_TYPE..."
cmake -B $BUILD_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# Build the project
echo "Building project ($BUILD_TYPE)..."
cmake --build $BUILD_DIR --config $BUILD_TYPE -j$NUM_CORES

# Validate output directories
echo "Validating $BUILD_TYPE output directories..."
if [ ! -d "$BUILD_DIR/$BUILD_TYPE/bin" ]; then
    echo "Error: $BUILD_TYPE/bin directory not found!"
    exit 1
fi

# Run tests if requested
if [ $RUN_TESTS -eq 1 ]; then
    echo "Running $BUILD_TYPE tests..."
    cd $BUILD_DIR
    ctest --output-on-failure --build-config $BUILD_TYPE
    cd ..
fi

# Run DemoApp if requested and tests passed
if [ $RUN_DEMO -eq 1 ]; then
    echo "Running DemoApp..."
    cd $BUILD_DIR/$BUILD_TYPE/bin
    ./DemoApp
    cd ../../..
fi

echo "=== $BUILD_TYPE build completed successfully! ==="