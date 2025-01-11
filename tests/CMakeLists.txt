# Download and configure Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Add test executable
add_executable(native_hosting_tests
    native_hosting_tests.cpp
)

# Link against Google Test and our library
target_link_libraries(native_hosting_tests
    PRIVATE
    NativeHosting
    GTest::gtest_main
)

if(APPLE)
    target_link_libraries(native_hosting_tests PRIVATE "${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
endif()

# Include directories
target_include_directories(native_hosting_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/native
    ${DOTNET_HOSTING_INCLUDE_PATH}
)

# Add test to CTest
include(GoogleTest)
gtest_discover_tests(native_hosting_tests) 