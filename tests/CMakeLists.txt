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
    GTest::gtest_main
    native_aot_plugin_host
)

if(APPLE)
    target_link_libraries(native_hosting_tests PRIVATE "${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
endif()

# Include directories
target_include_directories(native_hosting_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/native_aot_plugin_host
    ${DOTNET_HOSTING_INCLUDE_PATH}
)

# Build TestLibrary project
add_custom_target(build_test_library ALL
    COMMAND dotnet build ${CMAKE_SOURCE_DIR}/tests/TestLibrary/TestLibrary.csproj -c Release
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Copy TestLibrary files to output directory
add_custom_command(TARGET native_hosting_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/tests/TestLibrary/bin/Release/net9.0/TestLibrary.dll
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TestLibrary.dll
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/tests/TestLibrary/bin/Release/net9.0/TestLibrary.runtimeconfig.json
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TestLibrary.runtimeconfig.json
)

# Make sure TestLibrary is built before the tests
add_dependencies(native_hosting_tests build_test_library)

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

