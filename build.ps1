# Exit on error
$ErrorActionPreference = "Stop"

# Remove existing build directory
if (Test-Path build) {
    Remove-Item -Recurse -Force build
}

# Detect OS and set runtime identifier
$OS = switch ($PSVersionTable.Platform) {
    "Unix" {
        if ($IsMacOS) {
            "macos"
        } else {
            "linux"
        }
    }
    "Win32NT" { "windows" }
    default { throw "Unsupported platform" }
}

$RUNTIME_ID = switch ($OS) {
    "macos" { "osx-arm64" }
    "linux" { "linux-x64" }
    "windows" { "win-x64" }
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
    "windows" { "$env:ProgramFiles\dotnet" }
}

Write-Host "Detected OS: $OS"
Write-Host "Runtime Identifier: $RUNTIME_ID"
Write-Host "DOTNET_ROOT: $DOTNET_ROOT"

# Store root directory
$ROOT_DIR = Get-Location

# Create build directory
New-Item -ItemType Directory -Force -Path build | Out-Null
Set-Location build

# Create output directory
$OUTPUT_DIR = "bin"
New-Item -ItemType Directory -Force -Path $OUTPUT_DIR | Out-Null

# Build native library and tests
Write-Host "Building native library and tests..."
cmake .. -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$OUTPUT_DIR" -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$OUTPUT_DIR"
cmake --build . --config Release

# Go back to root directory
Set-Location $ROOT_DIR

# Build test library
Write-Host "Building test library..."
Set-Location tests/TestLibrary
if (-not (Test-Path "TestLibrary.csproj")) {
    throw "Error: TestLibrary.csproj not found in $(Get-Location)"
}
dotnet publish -c Release -r $RUNTIME_ID -o "../../build/tests"
Set-Location $ROOT_DIR

# Run tests
Set-Location build
Write-Host "Running tests..."
ctest --verbose --output-on-failure
Set-Location $ROOT_DIR

# Build .NET libraries
Write-Host "Building .NET libraries..."

# Build NativeAotPluginHost
Set-Location src/NativeAotPluginHost
if (-not (Test-Path "NativeAotPluginHost.csproj")) {
    throw "Error: NativeAotPluginHost.csproj not found in $(Get-Location)"
}
dotnet publish -c Release -r $RUNTIME_ID -o "../../../build/$OUTPUT_DIR"
Set-Location $ROOT_DIR

# Build ManagedLibrary
Set-Location src/ManagedLibrary
if (-not (Test-Path "ManagedLibrary.csproj")) {
    throw "Error: ManagedLibrary.csproj not found in $(Get-Location)"
}
dotnet publish -c Release -r $RUNTIME_ID -o "../../../build/$OUTPUT_DIR"
Set-Location $ROOT_DIR

# Build demo app
Write-Host "Building demo app..."
Set-Location src/DemoApp
if (-not (Test-Path "DemoApp.csproj")) {
    throw "Error: DemoApp.csproj not found in $(Get-Location)"
}
dotnet publish -c Release -r $RUNTIME_ID -o "../../build/$OUTPUT_DIR"
Set-Location $ROOT_DIR

Write-Host "Build completed successfully!"
Write-Host "All outputs can be found in: build/$OUTPUT_DIR"
Write-Host ""
Write-Host "Directory structure:"
Get-ChildItem "build/$OUTPUT_DIR" | Format-Table Name, Length, LastWriteTime 