# Stop on error
$ErrorActionPreference = "Stop"

# Set runtime identifier for Windows
$RuntimeId = "win-x64"
Write-Host "Runtime Identifier: $RuntimeId"

Write-Host "Building test library..."
# Build test library
Push-Location tests/TestLibrary
dotnet publish -c Release -r $RuntimeId
Pop-Location

Write-Host "Building native library and tests..."
# Create build directory for native code
New-Item -ItemType Directory -Force -Path build | Out-Null
Push-Location build
cmake ..
cmake --build . --config Release

Write-Host "Running tests..."
# Run tests
ctest --verbose --output-on-failure
Pop-Location

Write-Host "Building .NET library..."
# Build .NET library
Push-Location src/managed/DemoLibrary
dotnet publish -c Release -r $RuntimeId
Pop-Location

Write-Host "Building demo app..."
# Build demo app
Push-Location src/demo/DemoApp
dotnet publish -c Release -r $RuntimeId
Pop-Location

Write-Host "Copying native library to demo app..."
# Copy native library to demo app directory
$DemoAppDir = "src/demo/DemoApp/bin/Release/net8.0/$RuntimeId/publish"
New-Item -ItemType Directory -Force -Path $DemoAppDir | Out-Null
Copy-Item "build/bin/Release/NativeHosting.dll" -Destination $DemoAppDir

# Copy test artifacts to test directory
$TestDir = "build/tests"
New-Item -ItemType Directory -Force -Path $TestDir | Out-Null
Copy-Item "tests/TestLibrary/bin/Release/net8.0/$RuntimeId/publish/TestLibrary.dll" -Destination $TestDir
Copy-Item "tests/TestLibrary/bin/Release/net8.0/$RuntimeId/publish/TestLibrary.runtimeconfig.json" -Destination $TestDir

Write-Host "Build completed successfully!"
Write-Host "You can find the demo app in: $DemoAppDir" 