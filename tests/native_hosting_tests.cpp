#include <gtest/gtest.h>
#include "native_plugin_host.h"
#include <memory>
#include <string>
#include <climits>

using ReturnConstantDelegate = int32_t (*)();
using AddNumbersDelegate = int32_t (*)(int32_t, int32_t);

class NativeHostingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize paths
        assembly_path = "../tests/TestLibrary.dll";
        type_name = "TestLibrary.TestClass,TestLibrary";

        native_host_handle_t test_handle = nullptr;
        status = native_plugin_host_create(&test_handle);
        EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);
        EXPECT_NE(test_handle, nullptr);
        host_handle = test_handle;

        const char* configPath = "../tests/TestLibrary.runtimeconfig.json";
        native_plugin_handle_t test_plugin = nullptr;
        status = native_plugin_host_load(host_handle, configPath, &test_plugin);
        EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);
        plugin_handle = test_plugin;
    }

    void TearDown() override {
        if (plugin_handle != nullptr) {
            status = native_plugin_host_unload(host_handle, plugin_handle);
            EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);
        }
        if (host_handle != nullptr) {
            status = native_plugin_host_destroy(host_handle);
            EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);
        }
    }

    template<typename T>
    T get_function(const char* methodName, const char* delegateType) {
        void* fnPtr = nullptr;
        auto status = native_plugin_host_get_function_pointer(
            host_handle,
            plugin_handle,
            assembly_path.c_str(),
            type_name.c_str(),
            methodName,
            delegateType,
            &fnPtr);
        EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);
        EXPECT_NE(fnPtr, nullptr);
        return reinterpret_cast<T>(fnPtr);
    }

    native_host_handle_t host_handle = nullptr;
    native_plugin_handle_t plugin_handle = nullptr;
    NativePluginHostStatus status = NATIVE_PLUGIN_HOST_SUCCESS;
    std::string assembly_path;
    std::string type_name;
};

TEST_F(NativeHostingTest, CreateAndDestroyHost) {
    void* test_handle = nullptr;
    auto status = native_plugin_host_create(&test_handle);
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);
    EXPECT_NE(test_handle, nullptr);

    status = native_plugin_host_destroy(test_handle);
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);
}

TEST_F(NativeHostingTest, LoadAndUnloadPlugin) {
    void* test_handle = nullptr;
    void* test_plugin = nullptr;
    auto status = native_plugin_host_create(&test_handle);
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);

    const char* configPath = "../tests/TestLibrary.runtimeconfig.json";
    status = native_plugin_host_load(test_handle, configPath, &test_plugin);
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);

    status = native_plugin_host_unload(test_handle, test_plugin);
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);

    status = native_plugin_host_destroy(test_handle);
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);
}

TEST_F(NativeHostingTest, ReturnConstant) {
    auto fn = get_function<ReturnConstantDelegate>(
        "ReturnConstant",
        "TestLibrary.TestClass+ReturnConstantDelegate,TestLibrary"
    );
    EXPECT_NE(fn, nullptr);

    auto result = fn();
    EXPECT_EQ(result, 42);
}

TEST_F(NativeHostingTest, AddNumbers) {
    auto fn = get_function<AddNumbersDelegate>(
        "AddNumbers",
        "TestLibrary.TestClass+AddNumbersDelegate,TestLibrary"
    );
    EXPECT_NE(fn, nullptr);

    auto result = fn(40, 2);
    EXPECT_EQ(result, 42);
}

TEST_F(NativeHostingTest, BoundaryValues) {
    auto fn = get_function<AddNumbersDelegate>(
        "AddNumbers",
        "TestLibrary.TestClass+AddNumbersDelegate,TestLibrary"
    );
    EXPECT_NE(fn, nullptr);

    EXPECT_EQ(fn(INT32_MAX, 0), INT32_MAX);
    EXPECT_EQ(fn(INT32_MIN, 0), INT32_MIN);
    EXPECT_EQ(fn(0, INT32_MAX), INT32_MAX);
    EXPECT_EQ(fn(0, INT32_MIN), INT32_MIN);
}

TEST_F(NativeHostingTest, MultipleFunctionLoading) {
    auto fn1 = get_function<AddNumbersDelegate>(
        "AddNumbers",
        "TestLibrary.TestClass+AddNumbersDelegate,TestLibrary"
    );
    auto fn2 = get_function<AddNumbersDelegate>(
        "AddNumbers",
        "TestLibrary.TestClass+AddNumbersDelegate,TestLibrary"
    );
    EXPECT_NE(fn1, nullptr);
    EXPECT_NE(fn2, nullptr);

    EXPECT_EQ(fn1(40, 2), 42);
    EXPECT_EQ(fn2(40, 2), 42);
}

TEST_F(NativeHostingTest, InvalidAssemblyPath) {
    void* fnPtr = nullptr;
    auto status = native_plugin_host_get_function_pointer(
        host_handle,
        plugin_handle,
        "invalid.dll",
        "TestLibrary.TestClass,TestLibrary",
        "AddNumbers",
        "TestLibrary.TestClass+AddNumbersDelegate,TestLibrary",
        &fnPtr
    );
    EXPECT_NE(status, NATIVE_PLUGIN_HOST_SUCCESS);
    EXPECT_EQ(fnPtr, nullptr);
}

TEST_F(NativeHostingTest, InvalidTypeName) {
    void* fnPtr = nullptr;
    const char* assemblyPath = "../tests/TestLibrary.dll";
    auto status = native_plugin_host_get_function_pointer(
        host_handle,
        plugin_handle,
        assemblyPath,
        "InvalidType",
        "AddNumbers",
        "TestLibrary.TestClass+AddNumbersDelegate,TestLibrary",
        &fnPtr
    );
    EXPECT_NE(status, NATIVE_PLUGIN_HOST_SUCCESS);
    EXPECT_EQ(fnPtr, nullptr);
}

TEST_F(NativeHostingTest, InvalidMethodName) {
    void* fnPtr = nullptr;
    const char* assemblyPath = "../tests/TestLibrary.dll";
    auto status = native_plugin_host_get_function_pointer(
        host_handle,
        plugin_handle,
        assemblyPath,
        "TestLibrary.TestClass,TestLibrary",
        "InvalidMethod",
        "TestLibrary.TestClass+AddNumbersDelegate,TestLibrary",
        &fnPtr
    );
    EXPECT_NE(status, NATIVE_PLUGIN_HOST_SUCCESS);
    EXPECT_EQ(fnPtr, nullptr);
}

TEST_F(NativeHostingTest, SpecialCharactersInTypeName) {
    void* fnPtr = nullptr;
    const char* assemblyPath = "../tests/TestLibrary.dll";
    auto status = native_plugin_host_get_function_pointer(
        host_handle,
        plugin_handle,
        assemblyPath,
        "Test@Library.Test#Class,TestLibrary",
        "AddNumbers",
        "TestLibrary.TestClass+AddNumbersDelegate,TestLibrary",
        &fnPtr
    );
    EXPECT_NE(status, NATIVE_PLUGIN_HOST_SUCCESS);
    EXPECT_EQ(fnPtr, nullptr);
}

TEST_F(NativeHostingTest, MultipleMethodCalls) {
    auto fn = get_function<AddNumbersDelegate>(
        "AddNumbers",
        "TestLibrary.TestClass+AddNumbersDelegate,TestLibrary"
    );
    EXPECT_NE(fn, nullptr);

    for (int i = 0; i < 1000; i++) {
        EXPECT_EQ(fn(i, i), i * 2);
    }
}

TEST_F(NativeHostingTest, MultiplePluginLoadingSameFunction) {
    void* test_handle = nullptr;
    void* test_plugin = nullptr;
    auto status = native_plugin_host_create(&test_handle);
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);

    const char* configPath = "../tests/TestLibrary.runtimeconfig.json";
    status = native_plugin_host_load(test_handle, configPath, &test_plugin);
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);

    void* fnPtr1 = nullptr;
    void* fnPtr2 = nullptr;
    const char* assemblyPath = "../tests/TestLibrary.dll";
    const char* typeName = "TestLibrary.TestClass,TestLibrary";
    const char* methodName = "AddNumbers";
    const char* delegateType = "TestLibrary.TestClass+AddNumbersDelegate,TestLibrary";

    status = native_plugin_host_get_function_pointer(
        host_handle,
        plugin_handle,
        assemblyPath,
        typeName,
        methodName,
        delegateType,
        &fnPtr1
    );
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);
    EXPECT_NE(fnPtr1, nullptr);

    status = native_plugin_host_get_function_pointer(
        test_handle,
        test_plugin,
        assemblyPath,
        typeName,
        methodName,
        delegateType,
        &fnPtr2
    );
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);
    EXPECT_NE(fnPtr2, nullptr);

    auto fn1 = reinterpret_cast<AddNumbersDelegate>(fnPtr1);
    auto fn2 = reinterpret_cast<AddNumbersDelegate>(fnPtr2);

    EXPECT_EQ(fn1(40, 2), 42);
    EXPECT_EQ(fn2(40, 2), 42);

    status = native_plugin_host_unload(test_handle, test_plugin);
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);

    status = native_plugin_host_destroy(test_handle);
    EXPECT_EQ(status, NATIVE_PLUGIN_HOST_SUCCESS);
} 