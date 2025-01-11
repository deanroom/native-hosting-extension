# Stop on error
$ErrorActionPreference = "Stop"

Write-Host "Building native library..."
# Create build directory for native code
New-Item -ItemType Directory -Force -Path build | Out-Null
Push-Location build
cmake ..
cmake --build . --config Release
Pop-Location

Write-Host "Building .NET library..."
# Build .NET library
Push-Location src/managed/DemoLibrary
dotnet publish -c Release
Pop-Location

Write-Host "Building demo app..."
# Build demo app
Push-Location src/demo/DemoApp
dotnet publish -c Release
Pop-Location

Write-Host "Copying native library to demo app..."
# Copy native library to demo app directory
$DemoAppDir = "src/demo/DemoApp/bin/Release/net8.0/publish"
New-Item -ItemType Directory -Force -Path $DemoAppDir | Out-Null
Copy-Item "build/bin/Release/NativeHosting.dll" -Destination $DemoAppDir

Write-Host "Build completed successfully!"
Write-Host "You can find the demo app in: $DemoAppDir" 