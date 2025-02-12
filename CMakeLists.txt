if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

cmake_minimum_required(VERSION 3.20)
project(native-hosting-extension)

# Basic settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(DOTNET_SDK_VERSION "8.0.11" CACHE STRING ".NET SDK version")

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)

# Architecture detection
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(ARCH "x64")
else()
    message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Platform-specific settings
if(WIN32)
    set(HOST_ARCH "win-${ARCH}")
    set(PLATFORM_SUFFIX "win")
    set(CMAKE_GENERATOR_PLATFORM ${ARCH})
    set(DOTNET_ROOT "$ENV{ProgramFiles}/dotnet")
    if(NOT EXISTS ${DOTNET_ROOT})
        set(DOTNET_ROOT "$ENV{ProgramFiles(x86)}/dotnet")
    endif()
elseif(APPLE)
    set(HOST_ARCH "osx-${ARCH}")
    set(PLATFORM_SUFFIX "osx")
    set(CMAKE_OSX_ARCHITECTURES ${CMAKE_SYSTEM_PROCESSOR})
    if(EXISTS "/opt/homebrew/share/dotnet")
        set(DOTNET_ROOT "/opt/homebrew/share/dotnet")
    elseif(EXISTS "/usr/local/share/dotnet")
        set(DOTNET_ROOT "/usr/local/share/dotnet")
    else()
        message(FATAL_ERROR "Could not find .NET SDK in common locations")
    endif()
else()
    set(HOST_ARCH "linux-${ARCH}")
    set(PLATFORM_SUFFIX "linux")
    set(DOTNET_ROOT "/usr/share/dotnet")
endif()

# Verify .NET SDK installation
find_program(DOTNET_EXE dotnet)
if(NOT DOTNET_EXE)
    message(FATAL_ERROR ".NET SDK not found")
endif()

if(NOT EXISTS ${DOTNET_ROOT})
    message(FATAL_ERROR "DOTNET_ROOT not found at ${DOTNET_ROOT}")
endif()

# Verify and adjust SDK version
execute_process(
    COMMAND ${DOTNET_EXE} --version
    OUTPUT_VARIABLE DOTNET_VERSION_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
# if(NOT DOTNET_VERSION_OUTPUT MATCHES "^${DOTNET_SDK_VERSION}")
#     string(REGEX REPLACE "([0-9]+\\.[0-9]+).*" "\\1.0" DOTNET_SDK_VERSION ${DOTNET_VERSION_OUTPUT})
#     message(STATUS "Adjusting SDK version to ${DOTNET_SDK_VERSION}")
# endif()
# List files in DOTNET_HOSTING_BASE
file(GLOB DOTNET_HOSTING_FILES "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${PLATFORM_SUFFIX}-${ARCH}/*")
message(STATUS "Files in ${DOTNET_HOSTING_BASE}:")
foreach(FILE ${DOTNET_HOSTING_FILES})
    message(STATUS "  ${FILE}")
endforeach()

# Set .NET hosting paths
set(DOTNET_HOSTING_BASE "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.${PLATFORM_SUFFIX}-${ARCH}/${DOTNET_SDK_VERSION}/runtimes/${PLATFORM_SUFFIX}-${ARCH}/native")
set(DOTNET_HOSTING_INCLUDE_PATH ${DOTNET_HOSTING_BASE})
set(DOTNET_HOSTING_LIB_PATH ${DOTNET_HOSTING_BASE})

# Verify native hosting files
if(NOT EXISTS "${DOTNET_HOSTING_INCLUDE_PATH}/nethost.h")
    message(FATAL_ERROR "nethost.h not found at ${DOTNET_HOSTING_INCLUDE_PATH}")
endif()

# Build .NET projects
set(DOTNET_PROJECTS
    src/ManagedLibrary/ManagedLibrary.csproj
    src/ManagedLibrary3/ManagedLibrary3.csproj
)

add_custom_target(build_managed ALL)
foreach(PROJECT ${DOTNET_PROJECTS})
    add_custom_command(
        TARGET build_managed
        POST_BUILD
        COMMAND ${DOTNET_EXE} publish ${CMAKE_SOURCE_DIR}/${PROJECT} 
        -c $<CONFIG> 
        -o ${CMAKE_BINARY_DIR}/$<CONFIG>/bin
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Building ${PROJECT}"
    )
endforeach()

# Publish DemoApp to build directory
add_custom_command(
    TARGET build_managed
    POST_BUILD
    COMMAND ${DOTNET_EXE} publish ${CMAKE_SOURCE_DIR}/src/DemoApp/DemoApp.csproj 
        -c $<CONFIG> 
        -o ${CMAKE_BINARY_DIR}/$<CONFIG>/bin
        -r ${HOST_ARCH}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Publishing DemoApp"
)
# Add subdirectories
add_subdirectory(src/native_host)
add_subdirectory(tests)

# Enable testing
enable_testing()
