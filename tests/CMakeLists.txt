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
    COMMAND ${DOTNET_EXE} publish -c Release -r ${HOST_ARCH} -o ${CMAKE_BINARY_DIR}/tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/TestLibrary
)

# Define test sources
set(TEST_SOURCES
    native_host_basic_test.cpp
    native_host_assembly_test.cpp
    native_host_function_test.cpp
    native_host_concurrency_test.cpp
)

# Add test executable
add_executable(native_host_tests ${TEST_SOURCES})

# Copy runtime config for tests
add_custom_command(
    TARGET native_host_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/src/native_host/init.runtimeconfig.json"
        "$<TARGET_FILE_DIR:native_host_tests>/init.runtimeconfig.json"
    COMMENT "Copying init.runtimeconfig.json to test output directory"
)

# Set test properties
set_target_properties(native_host_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Link dependencies
target_link_libraries(native_host_tests PRIVATE native_host gtest gtest_main)
add_dependencies(native_host_tests build_test_library)

# Define test categories
set(TEST_CATEGORIES
    basic
    assembly
    function
    concurrency
)

# Add test category targets
foreach(CATEGORY ${TEST_CATEGORIES})
    add_custom_target(run_${CATEGORY}_tests
        COMMAND ${CMAKE_CTEST_COMMAND} -R "NativeHost${CATEGORY}Test.*" --output-on-failure
        DEPENDS native_host_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(native_host_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    PROPERTIES
        LABELS "all;unit"
        TIMEOUT 60
    DISCOVERY_TIMEOUT 60
    DISCOVERY_MODE PRE_TEST
)

# Set common test environment
set(TEST_ENV
    "PATH=${CMAKE_BINARY_DIR}/bin:$ENV{PATH}"
    "DOTNET_ROOT=${DOTNET_ROOT}"
)

# Add all tests target
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS native_host_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(
    NAME all_tests
    COMMAND native_host_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_tests_properties(all_tests PROPERTIES ENVIRONMENT "${TEST_ENV}")

