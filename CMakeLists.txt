cmake_minimum_required(VERSION 3.15)
project(NativeHostingExtension)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set architecture based on host system
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64")
        set(HOST_ARCH "win-x64")
        set(CMAKE_GENERATOR_PLATFORM "x64")
    else()
        set(HOST_ARCH "win-arm64")
        set(CMAKE_GENERATOR_PLATFORM "ARM64")
    endif()
    message(STATUS "Windows architecture: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
    message(STATUS "Selected HOST_ARCH: ${HOST_ARCH}")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(HOST_ARCH "osx-arm64")
    else()
        set(HOST_ARCH "osx-x64")
    endif()
    set(CMAKE_OSX_ARCHITECTURES "${CMAKE_HOST_SYSTEM_PROCESSOR}")
    message(STATUS "macOS architecture: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
    message(STATUS "Selected HOST_ARCH: ${HOST_ARCH}")
else()
    set(HOST_ARCH "linux-x64")
endif()

# Find .NET Core SDK path
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(DOTNET_ROOT $ENV{ProgramFiles}/dotnet)
    # Additional check for x86 Program Files
    if(NOT EXISTS ${DOTNET_ROOT})
        set(DOTNET_ROOT "$ENV{ProgramFiles(x86)}/dotnet")
    endif()
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    # Check for Homebrew installation first
    if(EXISTS "/opt/homebrew/share/dotnet")
        set(DOTNET_ROOT "/opt/homebrew/share/dotnet")
    elseif(EXISTS "/usr/local/share/dotnet")
        set(DOTNET_ROOT "/usr/local/share/dotnet")
    else()
        message(FATAL_ERROR "Could not find .NET SDK in common locations")
    endif()
    message(STATUS "Found .NET SDK at: ${DOTNET_ROOT}")
else()
    set(DOTNET_ROOT "/usr/share/dotnet")
endif()

# Verify DOTNET_ROOT exists
if(NOT EXISTS ${DOTNET_ROOT})
    message(FATAL_ERROR "DOTNET_ROOT not found at ${DOTNET_ROOT}")
endif()

message(STATUS "Using .NET SDK from: ${DOTNET_ROOT}")

# Set include and library paths
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(DOTNET_HOSTING_INCLUDE_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}/9.0.0/runtimes/${HOST_ARCH}/native")
    set(DOTNET_HOSTING_LIB_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}/9.0.0/runtimes/${HOST_ARCH}/native")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    # On macOS, we need to look in both the packs directory and the shared runtime directory
    set(DOTNET_HOSTING_INCLUDE_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}/9.0.0/runtimes/${HOST_ARCH}/native")
    
    # Check both possible locations for the library
    if(EXISTS "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}/9.0.0/runtimes/${HOST_ARCH}/native/libnethost.a")
        set(DOTNET_HOSTING_LIB_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}/9.0.0/runtimes/${HOST_ARCH}/native")
    elseif(EXISTS "${DOTNET_ROOT}/shared/Microsoft.NETCore.App/9.0.0/libnethost.a")
        set(DOTNET_HOSTING_LIB_PATH "${DOTNET_ROOT}/shared/Microsoft.NETCore.App/9.0.0")
    else()
        message(FATAL_ERROR "Could not find libnethost.a in either packs or shared runtime directory")
    endif()
else()
    set(DOTNET_HOSTING_INCLUDE_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}/9.0.0/runtimes/${HOST_ARCH}/native")
    set(DOTNET_HOSTING_LIB_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}/9.0.0/runtimes/${HOST_ARCH}/native")
endif()

if(NOT EXISTS ${DOTNET_HOSTING_INCLUDE_PATH})
    message(FATAL_ERROR "Include path not found at ${DOTNET_HOSTING_INCLUDE_PATH}")
endif()

if(NOT EXISTS ${DOTNET_HOSTING_LIB_PATH})
    message(FATAL_ERROR "Library path not found at ${DOTNET_HOSTING_LIB_PATH}")
endif()

message(STATUS "Using include path: ${DOTNET_HOSTING_INCLUDE_PATH}")
message(STATUS "Using library path: ${DOTNET_HOSTING_LIB_PATH}")

# List files in the library path for debugging
message(STATUS "Files in library path:")
file(GLOB LIB_FILES "${DOTNET_HOSTING_LIB_PATH}/*")
foreach(FILE ${LIB_FILES})
    message(STATUS "  ${FILE}")
endforeach()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add subdirectories
add_subdirectory(src/native_aot_plugin_host)
add_subdirectory(tests)