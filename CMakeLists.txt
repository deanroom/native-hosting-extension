cmake_minimum_required(VERSION 3.20)
project(native-hosting-extension)

# Set .NET SDK version
set(DOTNET_SDK_VERSION "9.0.0" CACHE STRING ".NET SDK version")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Detect host architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(HOST_ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(HOST_ARCH "x64")
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Find .NET SDK
find_program(DOTNET_EXE dotnet)
if(NOT DOTNET_EXE)
    message(FATAL_ERROR ".NET SDK not found")
endif()

# Verify .NET SDK version
execute_process(
    COMMAND ${DOTNET_EXE} --version
    OUTPUT_VARIABLE DOTNET_VERSION_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT DOTNET_VERSION_OUTPUT MATCHES "^${DOTNET_SDK_VERSION}")
    message(WARNING "Installed .NET SDK version (${DOTNET_VERSION_OUTPUT}) does not match required version (${DOTNET_SDK_VERSION})")
endif()

# Find .NET hosting bundle
if(WIN32)
    set(DOTNET_ROOT "$ENV{ProgramFiles}/dotnet")
elseif(APPLE)
    set(DOTNET_ROOT "/usr/local/share/dotnet")
else()
    set(DOTNET_ROOT "/usr/share/dotnet")
endif()

# Set .NET hosting paths
if(WIN32)
    set(DOTNET_HOSTING_LIB_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}-windows/${DOTNET_SDK_VERSION}/runtimes/win-${HOST_ARCH}/native")
elseif(APPLE)
    set(DOTNET_HOSTING_LIB_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.osx-${HOST_ARCH}/${DOTNET_SDK_VERSION}/runtimes/osx-${HOST_ARCH}/native")
else()
    set(DOTNET_HOSTING_LIB_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.linux-${HOST_ARCH}/${DOTNET_SDK_VERSION}/runtimes/linux-${HOST_ARCH}/native")
endif()

# Set include path based on current platform
if(WIN32)
    set(DOTNET_HOSTING_INCLUDE_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}-windows/${DOTNET_SDK_VERSION}/runtimes/win-${HOST_ARCH}/native")
elseif(APPLE)
    set(DOTNET_HOSTING_INCLUDE_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.osx-${HOST_ARCH}/${DOTNET_SDK_VERSION}/runtimes/osx-${HOST_ARCH}/native")
else()
    set(DOTNET_HOSTING_INCLUDE_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.linux-${HOST_ARCH}/${DOTNET_SDK_VERSION}/runtimes/linux-${HOST_ARCH}/native")
endif()

# Verify native hosting files exist
if(NOT EXISTS "${DOTNET_HOSTING_INCLUDE_PATH}/nethost.h")
    message(FATAL_ERROR "nethost.h not found at ${DOTNET_HOSTING_INCLUDE_PATH}")
endif()

if(NOT EXISTS "${DOTNET_HOSTING_LIB_PATH}/libnethost.a" AND NOT EXISTS "${DOTNET_HOSTING_LIB_PATH}/nethost.lib")
    message(FATAL_ERROR "nethost library not found at ${DOTNET_HOSTING_LIB_PATH}")
endif()

# Build .NET projects
add_custom_target(build_managed ALL
    COMMAND ${DOTNET_EXE} build ${CMAKE_SOURCE_DIR}/src/ManagedLibrary/ManagedLibrary.csproj -c Release
    COMMAND ${DOTNET_EXE} build ${CMAKE_SOURCE_DIR}/src/NativeHost/NativeHost.csproj -c Release
    COMMAND ${DOTNET_EXE} build ${CMAKE_SOURCE_DIR}/src/DemoApp/DemoApp.csproj -c Release
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Add subdirectories
add_subdirectory(src/native_host)
add_subdirectory(tests)

# Enable testing
enable_testing()