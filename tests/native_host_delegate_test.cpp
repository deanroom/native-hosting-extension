#include <gtest/gtest.h>
#include "native_host.h"
#include "test_utils.h"
#include <climits>

using ReturnConstantDelegate = int32_t (*)();
using AddNumbersDelegate = int32_t (*)(int32_t, int32_t);

class NativeHostFunctionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        assembly_path_ = "../tests/TestLibrary.dll";
        type_name_ = "TestLibrary.TestClass,TestLibrary";

        status_ = native_host_create(&host_handle_);
        EXPECT_EQ(status_, NativeHostStatus::SUCCESS);
        EXPECT_NE(host_handle_, nullptr);

        status_ = native_host_initialize(host_handle_);
        EXPECT_EQ(status_, NativeHostStatus::SUCCESS);

        status_ = native_host_load_assembly(host_handle_, assembly_path_.c_str(), &assembly_handle_);
        EXPECT_EQ(status_, NativeHostStatus::SUCCESS);
    }

    void TearDown() override
    {
        if (assembly_handle_ != nullptr)
        {
            status_ = native_host_unload_assembly(host_handle_, assembly_handle_);
            EXPECT_EQ(status_, NativeHostStatus::SUCCESS);
        }
        if (host_handle_ != nullptr)
        {
            status_ = native_host_destroy(host_handle_);
            EXPECT_EQ(status_, NativeHostStatus::SUCCESS);
        }
    }

    template <typename T>
    T getFunctionPointer(const char *method_name)
    {
        void *fn_ptr = nullptr;
        auto status = native_host_get_delegate(
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

TEST_F(NativeHostFunctionTest, GetFunctionPointerSucceeds)
{
    void* fn_ptr = nullptr;
    auto status = native_host_get_delegate(
        host_handle_,
        assembly_handle_,
        type_name_.c_str(),
        "ReturnConstant",
        &fn_ptr);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    EXPECT_NE(fn_ptr, nullptr);
}

TEST_F(NativeHostFunctionTest, GetFunctionPointerFailsWithNullTypeName)
{
    void* fn_ptr = nullptr;
    auto status = native_host_get_delegate(
        host_handle_,
        assembly_handle_,
        nullptr,
        "ReturnConstant",
        &fn_ptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);
}

TEST_F(NativeHostFunctionTest, GetFunctionPointerFailsWithNullMethodName)
{
    void* fn_ptr = nullptr;
    auto status = native_host_get_delegate(
        host_handle_,
        assembly_handle_,
        type_name_.c_str(),
        nullptr,
        &fn_ptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);
}

TEST_F(NativeHostFunctionTest, GetFunctionPointerFailsWithEmptyTypeName)
{
    void* fn_ptr = nullptr;
    auto status = native_host_get_delegate(
        host_handle_,
        assembly_handle_,
        "",
        "ReturnConstant",
        &fn_ptr);
    EXPECT_NE(status, NativeHostStatus::SUCCESS);
}

TEST_F(NativeHostFunctionTest, GetFunctionPointerFailsWithEmptyMethodName)
{
    void* fn_ptr = nullptr;
    auto status = native_host_get_delegate(
        host_handle_,
        assembly_handle_,
        type_name_.c_str(),
        "",
        &fn_ptr);
    EXPECT_NE(status, NativeHostStatus::SUCCESS);
}

TEST_F(NativeHostFunctionTest, ReturnConstantFunction)
{
    auto fn = getFunctionPointer<ReturnConstantDelegate>("ReturnConstant");
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn(), 42);
}

TEST_F(NativeHostFunctionTest, AddNumbersFunction)
{
    auto fn = getFunctionPointer<AddNumbersDelegate>("AddNumbers");
    EXPECT_NE(fn, nullptr);

    // Test normal cases
    EXPECT_EQ(fn(0, 0), 0);
    EXPECT_EQ(fn(1, 1), 2);
    EXPECT_EQ(fn(-1, 1), 0);
    EXPECT_EQ(fn(40, 2), 42);
}

TEST_F(NativeHostFunctionTest, AddNumbersBoundaryValues)
{
    auto fn = getFunctionPointer<AddNumbersDelegate>("AddNumbers");
    EXPECT_NE(fn, nullptr);

    // Test boundary cases
    EXPECT_EQ(fn(INT32_MAX, 0), INT32_MAX);
    EXPECT_EQ(fn(0, INT32_MAX), INT32_MAX);
    EXPECT_EQ(fn(INT32_MIN, 0), INT32_MIN);
    EXPECT_EQ(fn(0, INT32_MIN), INT32_MIN);
}

TEST_F(NativeHostFunctionTest, MultipleFunctionCalls)
{
    auto fn = getFunctionPointer<AddNumbersDelegate>("AddNumbers");
    EXPECT_NE(fn, nullptr);

    for (int i = 0; i < 1000; i++)
    {
        EXPECT_EQ(fn(i, i), i * 2);
    }
} 