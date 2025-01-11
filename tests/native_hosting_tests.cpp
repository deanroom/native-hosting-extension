#include <gtest/gtest.h>
#include "native_hosting.h"
#include <filesystem>
#include <string>

class NativeHostingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Get the current test directory
        auto currentPath = std::filesystem::current_path();
        testLibPath = (currentPath / "TestLibrary.dll").string();
        configPath = (currentPath / "TestLibrary.runtimeconfig.json").string();
        
        // Initialize runtime
        ASSERT_TRUE(initialize_runtime(configPath.c_str())) << "Failed to initialize runtime";
    }

    void TearDown() override {
        close_runtime();
    }

    std::string testLibPath;
    std::string configPath;
};

// Test initialization and cleanup
TEST_F(NativeHostingTest, InitializeAndCleanup) {
    // Already initialized in SetUp
    // Just verify we can close and reinitialize
    close_runtime();
    EXPECT_TRUE(initialize_runtime(configPath.c_str()));
}

// Test loading a method that returns a constant
TEST_F(NativeHostingTest, ReturnConstant) {
    using ReturnConstantDelegate = int(*)();
    
    void* fnPtr = load_assembly_and_get_function_pointer(
        testLibPath.c_str(),
        "TestLibrary.TestClass,TestLibrary",
        "ReturnConstant",
        "TestLibrary.ReturnConstantDelegate"
    );
    
    ASSERT_NE(fnPtr, nullptr);
    auto fn = reinterpret_cast<ReturnConstantDelegate>(fnPtr);
    EXPECT_EQ(fn(), 42);
}

// Test loading a method with parameters
TEST_F(NativeHostingTest, AddNumbers) {
    using AddNumbersDelegate = int(*)(int, int);
    
    void* fnPtr = load_assembly_and_get_function_pointer(
        testLibPath.c_str(),
        "TestLibrary.TestClass,TestLibrary",
        "AddNumbers",
        "TestLibrary.AddNumbersDelegate"
    );
    
    ASSERT_NE(fnPtr, nullptr);
    auto fn = reinterpret_cast<AddNumbersDelegate>(fnPtr);
    EXPECT_EQ(fn(5, 3), 8);
    EXPECT_EQ(fn(-1, 1), 0);
}

// Test error handling - invalid assembly path
TEST_F(NativeHostingTest, InvalidAssemblyPath) {
    void* fnPtr = load_assembly_and_get_function_pointer(
        "NonExistentAssembly.dll",
        "TestLibrary.TestClass,TestLibrary",
        "ReturnConstant",
        "TestLibrary.ReturnConstantDelegate"
    );
    
    EXPECT_EQ(fnPtr, nullptr);
}

// Test error handling - invalid type name
TEST_F(NativeHostingTest, InvalidTypeName) {
    void* fnPtr = load_assembly_and_get_function_pointer(
        testLibPath.c_str(),
        "TestLibrary.NonExistentClass,TestLibrary",
        "ReturnConstant",
        "TestLibrary.ReturnConstantDelegate"
    );
    
    EXPECT_EQ(fnPtr, nullptr);
}

// Test error handling - invalid method name
TEST_F(NativeHostingTest, InvalidMethodName) {
    void* fnPtr = load_assembly_and_get_function_pointer(
        testLibPath.c_str(),
        "TestLibrary.TestClass,TestLibrary",
        "NonExistentMethod",
        "TestLibrary.ReturnConstantDelegate"
    );
    
    EXPECT_EQ(fnPtr, nullptr);
}

// Test multiple method calls
TEST_F(NativeHostingTest, MultipleMethodCalls) {
    using AddNumbersDelegate = int(*)(int, int);
    
    void* fnPtr = load_assembly_and_get_function_pointer(
        testLibPath.c_str(),
        "TestLibrary.TestClass,TestLibrary",
        "AddNumbers",
        "TestLibrary.AddNumbersDelegate"
    );
    
    ASSERT_NE(fnPtr, nullptr);
    auto fn = reinterpret_cast<AddNumbersDelegate>(fnPtr);
    
    // Call the method multiple times
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(fn(i, i), i * 2);
    }
} 