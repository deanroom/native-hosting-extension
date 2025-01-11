cmake_minimum_required(VERSION 3.15)
project(NativeHostingExtension)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find .NET Core SDK path
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(DOTNET_ROOT $ENV{ProgramFiles}/dotnet)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    # Check for Homebrew installation first
    if(EXISTS "/opt/homebrew/share/dotnet")
        set(DOTNET_ROOT "/opt/homebrew/share/dotnet")
    else()
        set(DOTNET_ROOT "/usr/local/share/dotnet")
    endif()
else()
    set(DOTNET_ROOT "/usr/share/dotnet")
endif()

# Verify DOTNET_ROOT exists
if(NOT EXISTS ${DOTNET_ROOT})
    message(FATAL_ERROR "DOTNET_ROOT not found at ${DOTNET_ROOT}")
endif()

message(STATUS "Using .NET SDK from: ${DOTNET_ROOT}")

# Find SDK version
file(GLOB SDK_VERSIONS "${DOTNET_ROOT}/sdk/*")
if(NOT SDK_VERSIONS)
    message(FATAL_ERROR "No .NET SDK versions found in ${DOTNET_ROOT}/sdk")
endif()

list(SORT SDK_VERSIONS ORDER DESCENDING)
list(GET SDK_VERSIONS 0 SDK_VERSION)
message(STATUS "Using .NET SDK version: ${SDK_VERSION}")

# Set host architecture
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(HOST_ARCH "win-x64")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(HOST_ARCH "osx-arm64")
    else()
        set(HOST_ARCH "osx-x64")
    endif()
else()
    set(HOST_ARCH "linux-x64")
endif()

# Find hosting files
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    # On macOS, we need to look in the runtime directory
    set(DOTNET_HOST_PATH "${DOTNET_ROOT}/shared/Microsoft.NETCore.App")
else()
    set(DOTNET_HOST_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}")
endif()

if(NOT EXISTS ${DOTNET_HOST_PATH})
    message(FATAL_ERROR "Host pack not found at ${DOTNET_HOST_PATH}")
endif()

file(GLOB DOTNET_HOST_VERSIONS "${DOTNET_HOST_PATH}/*")
if(NOT DOTNET_HOST_VERSIONS)
    message(FATAL_ERROR "No host versions found in ${DOTNET_HOST_PATH}")
endif()

list(SORT DOTNET_HOST_VERSIONS ORDER DESCENDING)
list(GET DOTNET_HOST_VERSIONS 0 DOTNET_HOST_VERSION)
message(STATUS "Using .NET host version: ${DOTNET_HOST_VERSION}")

# Set include and library paths
set(DOTNET_HOSTING_INCLUDE_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}/8.0.0/runtimes/${HOST_ARCH}/native")
set(DOTNET_HOSTING_LIB_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}/8.0.0/runtimes/${HOST_ARCH}/native")

if(NOT EXISTS ${DOTNET_HOSTING_INCLUDE_PATH})
    message(FATAL_ERROR "Include path not found at ${DOTNET_HOSTING_INCLUDE_PATH}")
endif()

if(NOT EXISTS ${DOTNET_HOSTING_LIB_PATH})
    message(FATAL_ERROR "Library path not found at ${DOTNET_HOSTING_LIB_PATH}")
endif()

message(STATUS "Using include path: ${DOTNET_HOSTING_INCLUDE_PATH}")
message(STATUS "Using library path: ${DOTNET_HOSTING_LIB_PATH}")

# Enable testing
enable_testing()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories
add_subdirectory(src/native)
add_subdirectory(tests) 