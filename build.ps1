# Exit on error
$ErrorActionPreference = "Stop"

# Remove existing build directory
if (Test-Path build) {
    Remove-Item -Recurse -Force build
}

# Detect OS and set runtime identifier
$OS = if ($IsWindows -or ($PSVersionTable.PSVersion.Major -lt 6)) {
    "windows"
} elseif ($IsMacOS) {
    "macos"
} elseif ($IsLinux) {
    "linux"
} else {
    throw "Unsupported platform"
}

$RUNTIME_ID = switch ($OS) {
    "macos" { "osx-arm64" }
    "linux" { "linux-x64" }
    "windows" { 
        # Check if running on ARM64 Windows
        if ([System.Environment]::GetEnvironmentVariable("PROCESSOR_ARCHITECTURE") -eq "ARM64") {
            "win-arm64"
        } else {
            "win-x64"
        }
    }
}

$DOTNET_ROOT = switch ($OS) {
    "macos" {
        if (Test-Path "/opt/homebrew/share/dotnet") {
            "/opt/homebrew/share/dotnet"
        } else {
            "/usr/local/share/dotnet"
        }
    }
    "linux" { "/usr/share/dotnet" }
    "windows" { 
        if (Test-Path "${env:ProgramFiles}\dotnet") {
            "${env:ProgramFiles}\dotnet"
        } elseif (Test-Path "${env:ProgramFiles(x86)}\dotnet") {
            "${env:ProgramFiles(x86)}\dotnet"
        } else {
            throw ".NET SDK not found in Program Files"
        }
    }
}

# Convert paths to platform-specific format
$BUILD_DIR = Join-Path $PWD "build"
$OUTPUT_DIR = "bin"

Write-Host "Detected OS: $OS"
Write-Host "Runtime Identifier: $RUNTIME_ID"
Write-Host "DOTNET_ROOT: $DOTNET_ROOT"

# Store root directory
$ROOT_DIR = $PWD

# Create build directory
New-Item -ItemType Directory -Force -Path $BUILD_DIR | Out-Null
Set-Location $BUILD_DIR

# Create output directory
$FULL_OUTPUT_DIR = Join-Path $BUILD_DIR $OUTPUT_DIR
New-Item -ItemType Directory -Force -Path $FULL_OUTPUT_DIR | Out-Null

# Build native library and tests
Write-Host "Building native library and tests..."
cmake .. "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$FULL_OUTPUT_DIR" "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=$FULL_OUTPUT_DIR"
cmake --build . --config Release

# Go back to root directory
Set-Location $ROOT_DIR

# Build test library
Write-Host "Building test library..."
Set-Location (Join-Path "tests" "TestLibrary")
if (-not (Test-Path "TestLibrary.csproj")) {
    throw "Error: TestLibrary.csproj not found in $(Get-Location)"
}
$TEST_OUTPUT = Join-Path (Join-Path $ROOT_DIR "build") "tests"
dotnet publish -c Release -r $RUNTIME_ID -o $TEST_OUTPUT
Set-Location $ROOT_DIR

# Run tests
Set-Location $BUILD_DIR
Write-Host "Running tests..."
./native_hosting_tests.exe
Set-Location $ROOT_DIR

# Build .NET libraries
Write-Host "Building .NET libraries..."

# Build NativeAotPluginHost
Set-Location (Join-Path "src" "NativeAotPluginHost")
if (-not (Test-Path "NativeAotPluginHost.csproj")) {
    throw "Error: NativeAotPluginHost.csproj not found in $(Get-Location)"
}
$LIB_OUTPUT = Join-Path (Join-Path $ROOT_DIR "build") $OUTPUT_DIR
dotnet publish -c Release -r $RUNTIME_ID -o $LIB_OUTPUT
Set-Location $ROOT_DIR

# Build ManagedLibrary
Set-Location (Join-Path "src" "ManagedLibrary")
if (-not (Test-Path "ManagedLibrary.csproj")) {
    throw "Error: ManagedLibrary.csproj not found in $(Get-Location)"
}
dotnet publish -c Release -r $RUNTIME_ID -o $LIB_OUTPUT
Set-Location $ROOT_DIR

# Build demo app
Write-Host "Building demo app..."
Set-Location (Join-Path "src" "DemoApp")
if (-not (Test-Path "DemoApp.csproj")) {
    throw "Error: DemoApp.csproj not found in $(Get-Location)"
}
$DEMO_OUTPUT = Join-Path (Join-Path $ROOT_DIR "build") $OUTPUT_DIR
dotnet publish -c Release -r $RUNTIME_ID -o $DEMO_OUTPUT
Set-Location $ROOT_DIR

Write-Host "Build completed successfully!"
Write-Host "All outputs can be found in: build/$OUTPUT_DIR"
Write-Host ""
Write-Host "Directory structure:"
Get-ChildItem (Join-Path "build" $OUTPUT_DIR) | Format-Table Name, Length, LastWriteTime 