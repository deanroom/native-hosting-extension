# Download and configure Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Add test executable
add_executable(native_hosting_tests
    native_hosting_tests.cpp
)

# Set the output directory for the test executable
set_target_properties(native_hosting_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

# Link against Google Test and our library
target_link_libraries(native_hosting_tests
    PRIVATE
    gtest
    gtest_main
    native_plugin_host
    ${DOTNET_LIBRARY_PATH}/libnethost.a
    dl
)

# Include directories
target_include_directories(native_hosting_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/native_plugin_host
    ${DOTNET_HOSTING_INCLUDE_PATH}
)

# Add test library build target
add_custom_target(test_library
    COMMAND dotnet build -c Release "${CMAKE_SOURCE_DIR}/tests/TestLibrary/TestLibrary.csproj" -o "${CMAKE_BINARY_DIR}/tests"
    COMMENT "Building test library..."
)

add_dependencies(native_hosting_tests test_library)

# Add the test directly to CTest
add_test(
    NAME native_hosting_tests
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/native_hosting_tests
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Set test environment
set_tests_properties(native_hosting_tests PROPERTIES
    ENVIRONMENT "PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}:$ENV{PATH}"
)

