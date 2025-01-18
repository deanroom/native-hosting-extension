#include <gtest/gtest.h>
#include "native_host.h"
#include <memory>
#include <string>
#include <climits>

using ReturnConstantDelegate = int32_t (*)();
using AddNumbersDelegate = int32_t (*)(int32_t, int32_t);

class NativeHostTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize paths
        assembly_path_ = "../tests/TestLibrary.dll";
        type_name_ = "TestLibrary.TestClass,TestLibrary";

        // Skip host creation for CreateAndDestroyHost test
        if (::testing::UnitTest::GetInstance()->current_test_info()->name() == std::string("CreateAndDestroyHost"))
        {
            return;
        }

        native_host_handle_t test_handle = nullptr;
        status_ = create(&test_handle);
        EXPECT_EQ(status_, NativeHostStatus::SUCCESS);
        EXPECT_NE(test_handle, nullptr);
        host_handle_ = test_handle;

        // Initialize runtime
        status_ = initialize(host_handle_);
        EXPECT_EQ(status_, NativeHostStatus::SUCCESS);

        // Load assembly
        native_assembly_handle_t test_assembly = nullptr;
        status_ = load(host_handle_, assembly_path_.c_str(), &test_assembly);
        EXPECT_EQ(status_, NativeHostStatus::SUCCESS);
        assembly_handle_ = test_assembly;
    }

    void TearDown() override
    {
        // Skip cleanup for CreateAndDestroyHost test
        if (::testing::UnitTest::GetInstance()->current_test_info()->name() == std::string("CreateAndDestroyHost"))
        {
            return;
        }

        if (assembly_handle_ != nullptr)
        {
            status_ = unload(host_handle_, assembly_handle_);
            EXPECT_EQ(status_, NativeHostStatus::SUCCESS);
        }
        if (host_handle_ != nullptr)
        {
            status_ = destroy(host_handle_);
            EXPECT_EQ(status_, NativeHostStatus::SUCCESS);
        }
    }

    template <typename T>
    T getFunctionPointer(const char *method_name)
    {
        void *fn_ptr = nullptr;
        auto status = get_function_pointer(
            host_handle_,
            assembly_handle_,
            type_name_.c_str(),
            method_name,
            &fn_ptr);
        EXPECT_EQ(status, NativeHostStatus::SUCCESS);
        EXPECT_NE(fn_ptr, nullptr);
        return reinterpret_cast<T>(fn_ptr);
    }

    native_host_handle_t host_handle_ = nullptr;
    native_assembly_handle_t assembly_handle_ = nullptr;
    NativeHostStatus status_ = NativeHostStatus::SUCCESS;
    std::string assembly_path_;
    std::string type_name_;
};

TEST_F(NativeHostTest, CreateAndDestroyHost)
{
    // First create should succeed
    void *test_handle = nullptr;
    auto status = create(&test_handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    EXPECT_NE(test_handle, nullptr);

    // Second create should fail with ERROR_HOST_ALREADY_EXISTS
    void *second_handle = nullptr;
    status = create(&second_handle);
    EXPECT_EQ(status, NativeHostStatus::ERROR_HOST_ALREADY_EXISTS);

    // Destroy should succeed
    status = destroy(test_handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    // After destroy, create should succeed again
    status = create(&test_handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    EXPECT_NE(test_handle, nullptr);

    status = destroy(test_handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
}

TEST_F(NativeHostTest, LoadAndUnloadassembly)
{
    // Use the host created in SetUp()
    void *test_assembly = nullptr;
    const char *config_path = "../tests/TestLibrary.runtimeconfig.json";
    auto status = load(host_handle_, config_path, &test_assembly);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    status = unload(host_handle_, test_assembly);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
}

TEST_F(NativeHostTest, ReturnConstant)
{
    auto fn = getFunctionPointer<ReturnConstantDelegate>(
        "ReturnConstant");
    EXPECT_NE(fn, nullptr);

    auto result = fn();
    EXPECT_EQ(result, 42);
}

TEST_F(NativeHostTest, AddNumbers)
{
    auto fn = getFunctionPointer<AddNumbersDelegate>(
        "AddNumbers");
    EXPECT_NE(fn, nullptr);

    auto result = fn(40, 2);
    EXPECT_EQ(result, 42);
}

TEST_F(NativeHostTest, BoundaryValues)
{
    auto fn = getFunctionPointer<AddNumbersDelegate>(
        "AddNumbers");
    EXPECT_NE(fn, nullptr);

    EXPECT_EQ(fn(INT32_MAX, 0), INT32_MAX);
    EXPECT_EQ(fn(INT32_MIN, 0), INT32_MIN);
    EXPECT_EQ(fn(0, INT32_MAX), INT32_MAX);
    EXPECT_EQ(fn(0, INT32_MIN), INT32_MIN);
}

TEST_F(NativeHostTest, MultipleFunctionLoading)
{
    auto fn1 = getFunctionPointer<AddNumbersDelegate>(
        "AddNumbers");
    auto fn2 = getFunctionPointer<AddNumbersDelegate>(
        "AddNumbers");
    EXPECT_NE(fn1, nullptr);
    EXPECT_NE(fn2, nullptr);

    EXPECT_EQ(fn1(40, 2), 42);
    EXPECT_EQ(fn2(40, 2), 42);
}

TEST_F(NativeHostTest, InvalidAssemblyPath)
{
    void *fn_ptr = nullptr;
    auto status = get_function_pointer(
        host_handle_,
        assembly_handle_,
        "InvalidType",
        "AddNumbers",
        &fn_ptr);
    EXPECT_NE(status, NativeHostStatus::SUCCESS);
    EXPECT_EQ(fn_ptr, nullptr);
}

TEST_F(NativeHostTest, InvalidTypeName)
{
    void *fn_ptr = nullptr;
    auto status = get_function_pointer(
        host_handle_,
        assembly_handle_,
        "InvalidType",
        "AddNumbers",
        &fn_ptr);
    EXPECT_NE(status, NativeHostStatus::SUCCESS);
    EXPECT_EQ(fn_ptr, nullptr);
}

TEST_F(NativeHostTest, InvalidMethodName)
{
    void *fn_ptr = nullptr;
    auto status = get_function_pointer(
        host_handle_,
        assembly_handle_,
        type_name_.c_str(),
        "InvalidMethod",
        &fn_ptr);
    EXPECT_NE(status, NativeHostStatus::SUCCESS);
    EXPECT_EQ(fn_ptr, nullptr);
}

TEST_F(NativeHostTest, SpecialCharactersInTypeName)
{
    void *fn_ptr = nullptr;
    auto status = get_function_pointer(
        host_handle_,
        assembly_handle_,
        "Test@Library.Test#Class,TestLibrary",
        "AddNumbers",
        &fn_ptr);
    EXPECT_NE(status, NativeHostStatus::SUCCESS);
    EXPECT_EQ(fn_ptr, nullptr);
}

TEST_F(NativeHostTest, MultipleMethodCalls)
{
    auto fn = getFunctionPointer<AddNumbersDelegate>(
        "AddNumbers");
    EXPECT_NE(fn, nullptr);

    for (int i = 0; i < 1000; i++)
    {
        EXPECT_EQ(fn(i, i), i * 2);
    }
}

TEST_F(NativeHostTest, MultipleassemblyLoadingSameFunction)
{
    // Use the host created in SetUp()
    void *test_assembly2 = nullptr;
    auto status = load(host_handle_, assembly_path_.c_str(), &test_assembly2);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    // Get function pointers from both assembly instances
    void *fn_ptr1 = nullptr;
    status = get_function_pointer(
        host_handle_,
        assembly_handle_,
        type_name_.c_str(),
        "ReturnConstant",
        &fn_ptr1);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    EXPECT_NE(fn_ptr1, nullptr);

    void *fn_ptr2 = nullptr;
    status = get_function_pointer(
        host_handle_,
        test_assembly2,
        type_name_.c_str(),
        "ReturnConstant",
        &fn_ptr2);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    EXPECT_NE(fn_ptr2, nullptr);

    // Both function pointers should work
    auto fn1 = reinterpret_cast<ReturnConstantDelegate>(fn_ptr1);
    auto fn2 = reinterpret_cast<ReturnConstantDelegate>(fn_ptr2);

    EXPECT_EQ(fn1(), 42);
    EXPECT_EQ(fn2(), 42);

    // Clean up the second assembly
    status = unload(host_handle_, test_assembly2);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
}