#!/bin/bash
set -e

# Default values
BUILD_TYPE="Debug"
BUILD_DIR="build"
CLEAN=1
RUN_TESTS=1

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
        --clean)
            CLEAN=1
            shift
            ;;
        --test)
            RUN_TESTS=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--debug] [--clean] [--test]"
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

# Configure CMake
echo "Configuring CMake..."
cmake -B $BUILD_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# Build the project
echo "Building project..."
cmake --build $BUILD_DIR --config $BUILD_TYPE -j$NUM_CORES

# Run tests if requested
if [ $RUN_TESTS -eq 1 ]; then
    echo "Running tests..."
    cd $BUILD_DIR
    ctest --output-on-failure --build-config $BUILD_TYPE
    cd ..
fi

echo "Build completed successfully!"