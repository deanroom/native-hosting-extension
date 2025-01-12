#include <gtest/gtest.h>
#include "native_aot_plugin_host.h"
#include <filesystem>
#include <string>

class NativeHostingTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto currentPath = std::filesystem::current_path();
        testLibPath = (currentPath / "TestLibrary.dll").string();
        configPath = (currentPath / "TestLibrary.runtimeconfig.json").string();
        typeName = "TestLibrary.TestClass,TestLibrary";
        
        ASSERT_TRUE(initialize_runtime(configPath.c_str())) << "Failed to initialize runtime";
    }

    void TearDown() override {
        close_runtime();
    }

    template<typename T>
    T GetFunction(const char* methodName, const char* delegateType) {
        void* fnPtr = load_assembly_and_get_function_pointer(
            testLibPath.c_str(),
            typeName.c_str(),
            methodName,
            delegateType
        );
        EXPECT_NE(fnPtr, nullptr);
        return reinterpret_cast<T>(fnPtr);
    }

    std::string testLibPath;
    std::string configPath;
    std::string typeName;
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
    auto fn = GetFunction<ReturnConstantDelegate>(
        "ReturnConstant",
        "TestLibrary.ReturnConstantDelegate"
    );
    EXPECT_EQ(fn(), 42);
}

// Test loading a method with parameters
TEST_F(NativeHostingTest, AddNumbers) {
    using AddNumbersDelegate = int(*)(int, int);
    auto fn = GetFunction<AddNumbersDelegate>(
        "AddNumbers",
        "TestLibrary.AddNumbersDelegate"
    );
    EXPECT_EQ(fn(5, 3), 8);
    EXPECT_EQ(fn(-1, 1), 0);
}

// Test error handling - invalid assembly path
TEST_F(NativeHostingTest, InvalidAssemblyPath) {
    void* fnPtr = load_assembly_and_get_function_pointer(
        "NonExistentAssembly.dll",
        typeName.c_str(),
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
        typeName.c_str(),
        "NonExistentMethod",
        "TestLibrary.ReturnConstantDelegate"
    );
    EXPECT_EQ(fnPtr, nullptr);
}

// Test multiple method calls
TEST_F(NativeHostingTest, MultipleMethodCalls) {
    using AddNumbersDelegate = int(*)(int, int);
    auto fn = GetFunction<AddNumbersDelegate>(
        "AddNumbers",
        "TestLibrary.AddNumbersDelegate"
    );
    
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(fn(i, i), i * 2);
    }
} 