# Exit on error
$ErrorActionPreference = "Stop"

# Default values
$BUILD_DIR = "build"
$CLEAN = $true
$RUN_TESTS = $true
$RUN_DEMO = $true
$BUILD_TYPE = "Debug"

# Get number of CPU cores
$NUM_CORES = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
if (-not $NUM_CORES) { $NUM_CORES = 4 }  # Default to 4 if detection fails

# Parse command line arguments
foreach ($arg in $args) {
    switch ($arg) {
        "--debug" { $BUILD_TYPE = "Debug" }
        "--release" { $BUILD_TYPE = "Release" }
        "--clean" { $CLEAN = $true }
        "--test" { $RUN_TESTS = $true }
        "--demo" { $RUN_DEMO = $true }
        default {
            Write-Host "Unknown option: $arg"
            Write-Host "Usage: $PSCommandPath [--debug|--release] [--clean] [--test] [--demo]"
            exit 1
        }
    }
}

# Clean build directory if requested
if ($CLEAN) {
    Write-Host "Cleaning build directory..."
    if (Test-Path $BUILD_DIR) {
        Remove-Item -Recurse -Force $BUILD_DIR
    }
}

# Create build directory
New-Item -ItemType Directory -Force -Path $BUILD_DIR | Out-Null

Write-Host "=== Building $BUILD_TYPE configuration ==="

# Configure CMake
Write-Host "Configuring CMake for $BUILD_TYPE..."
cmake -B $BUILD_DIR "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

# Build the project
Write-Host "Building project ($BUILD_TYPE)..."
cmake --build $BUILD_DIR --config $BUILD_TYPE --parallel $NUM_CORES

# Validate output directories
Write-Host "Validating $BUILD_TYPE output directories..."
if (-not (Test-Path "$BUILD_DIR/$BUILD_TYPE/bin")) {
    Write-Host "Error: $BUILD_TYPE/bin directory not found!"
    exit 1
}

# Run tests if requested
if ($RUN_TESTS) {
    Write-Host "Running $BUILD_TYPE tests..."
    Push-Location $BUILD_DIR
    dir $BUILD_DIR/tests
    ctest --output-on-failure --build-config $BUILD_TYPE
    Pop-Location
}

# Run DemoApp if requested and tests passed
if ($RUN_DEMO) {
    Write-Host "Running DemoApp..."
    Push-Location "$BUILD_DIR/$BUILD_TYPE/bin"
    ./DemoApp
    Pop-Location
}

Write-Host "=== $BUILD_TYPE build completed successfully! ==="