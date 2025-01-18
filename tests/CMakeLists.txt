# Download and configure Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Build test library
add_custom_target(build_test_library
    COMMAND dotnet publish -c Release -r ${HOST_ARCH} -o ${CMAKE_BINARY_DIR}/tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/TestLibrary
)

# Add test executable
add_executable(native_host_tests
    native_host_basic_test.cpp
    native_host_assembly_test.cpp
    native_host_function_test.cpp
    native_host_concurrency_test.cpp
)

# Set output directory
set_target_properties(native_host_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

# Link dependencies
target_link_libraries(native_host_tests
    PRIVATE
    native_host
    gtest
    gtest_main
)

# Add dependencies
add_dependencies(native_host_tests build_test_library)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(native_host_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PROPERTIES
        LABELS "all;unit"
        TIMEOUT 60  # Timeout in seconds
    DISCOVERY_TIMEOUT 60
    DISCOVERY_MODE PRE_TEST
)

# Set environment variables for all tests
set_property(DIRECTORY PROPERTY ENVIRONMENT 
    "PATH=${CMAKE_BINARY_DIR}/bin:$ENV{PATH}"
    "DOTNET_ROOT=${DOTNET_ROOT}"
)

# Add custom targets for running test categories
add_custom_target(run_basic_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "NativeHostBasicTest.*" --output-on-failure
    DEPENDS native_host_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(run_assembly_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "NativeHostAssemblyTest.*" --output-on-failure
    DEPENDS native_host_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(run_function_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "NativeHostFunctionTest.*" --output-on-failure
    DEPENDS native_host_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(run_concurrency_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "NativeHostConcurrencyTest.*" --output-on-failure
    DEPENDS native_host_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS native_host_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Add a test that runs all tests directly
add_test(
    NAME all_tests
    COMMAND native_host_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(all_tests PROPERTIES
    ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin:$ENV{PATH};DOTNET_ROOT=${DOTNET_ROOT}"
)

