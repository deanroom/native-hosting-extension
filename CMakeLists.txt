cmake_minimum_required(VERSION 3.10)
project(native_hosting_extension)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find .NET SDK
if(NOT DEFINED DOTNET_ROOT)
    if(WIN32)
        set(DOTNET_ROOT "$ENV{ProgramFiles}/dotnet")
    else()
        set(DOTNET_ROOT "/usr/local/share/dotnet")
    endif()
endif()

if(NOT EXISTS ${DOTNET_ROOT})
    message(FATAL_ERROR ".NET SDK not found at: ${DOTNET_ROOT}")
endif()

message(STATUS "Found .NET SDK at: ${DOTNET_ROOT}")
message(STATUS "Using .NET SDK from: ${DOTNET_ROOT}")

# Detect host architecture
if(WIN32)
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64")
        set(HOST_ARCH "win-x64")
    else()
        set(HOST_ARCH "win-arm64")
    endif()
elseif(APPLE)
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(HOST_ARCH "osx-arm64")
    else()
        set(HOST_ARCH "osx-x64")
    endif()
    message(STATUS "macOS architecture: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
else()
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(HOST_ARCH "linux-x64")
    elseif(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(HOST_ARCH "linux-arm64")
    endif()
endif()

message(STATUS "Selected HOST_ARCH: ${HOST_ARCH}")

# Set paths for .NET hosting
set(DOTNET_VERSION "9.0.0")
set(DOTNET_RUNTIME_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${HOST_ARCH}/${DOTNET_VERSION}/runtimes/${HOST_ARCH}/native")

set(DOTNET_INCLUDE_PATH "${DOTNET_RUNTIME_PATH}")
set(DOTNET_LIBRARY_PATH "${DOTNET_RUNTIME_PATH}")

message(STATUS "Using include path: ${DOTNET_INCLUDE_PATH}")
message(STATUS "Using library path: ${DOTNET_LIBRARY_PATH}")

# List files in library path
message(STATUS "Files in library path:")
file(GLOB LIB_FILES "${DOTNET_LIBRARY_PATH}/*")
foreach(FILE ${LIB_FILES})
    message(STATUS "  ${FILE}")
endforeach()

# Add subdirectories
add_subdirectory(src/native_plugin_host)
add_subdirectory(tests)

# Custom target for building NativePluginHost
add_custom_target(plugin_host
    COMMAND dotnet build -c Release "${CMAKE_SOURCE_DIR}/src/NativePluginHost/NativePluginHost.csproj" -o "${CMAKE_BINARY_DIR}/bin"
    COMMENT "Building NativePluginHost..."
)

# Custom target for building ManagedLibrary
add_custom_target(managed_library
    COMMAND dotnet build -c Release "${CMAKE_SOURCE_DIR}/src/ManagedLibrary/ManagedLibrary.csproj" -o "${CMAKE_BINARY_DIR}/bin"
    COMMENT "Building ManagedLibrary..."
)

# Custom target for building demo app
add_custom_target(demo_app
    COMMAND dotnet build -c Release "${CMAKE_SOURCE_DIR}/src/DemoApp/DemoApp.csproj" -o "${CMAKE_BINARY_DIR}/bin"
    COMMENT "Building demo app..."
)

# Add custom target to run tests
add_custom_target(run_tests
    COMMAND ${CMAKE_BINARY_DIR}/bin/native_hosting_tests
    DEPENDS native_hosting_tests
    COMMENT "Running tests..."
)

# Add custom target to run demo app
add_custom_target(run_demo
    COMMAND ${CMAKE_BINARY_DIR}/bin/DemoApp
    DEPENDS demo_app
    COMMENT "Running demo app..."
)

# Copy .NET runtime libraries
if(WIN32)
    file(COPY "${DOTNET_LIBRARY_PATH}/nethost.dll" DESTINATION "${CMAKE_BINARY_DIR}/bin")
else()
    file(COPY "${DOTNET_LIBRARY_PATH}/libnethost.dylib" DESTINATION "${CMAKE_BINARY_DIR}/bin")
endif()