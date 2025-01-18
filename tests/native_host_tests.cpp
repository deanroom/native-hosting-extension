#include <gtest/gtest.h>
#include "native_host.h"
#include <memory>
#include <string>
#include <climits>
#include <thread>
#include <vector>
#include <chrono>

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

        // Skip host creation for specific tests
        if (::testing::UnitTest::GetInstance()->current_test_info()->name() == std::string("CreateAndDestroyHost") ||
            ::testing::UnitTest::GetInstance()->current_test_info()->name() == std::string("NullHandleOperations") ||
            ::testing::UnitTest::GetInstance()->current_test_info()->name() == std::string("RuntimeInitialization"))
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
        // Skip cleanup for specific tests
        if (::testing::UnitTest::GetInstance()->current_test_info()->name() == std::string("CreateAndDestroyHost") ||
            ::testing::UnitTest::GetInstance()->current_test_info()->name() == std::string("NullHandleOperations") ||
            ::testing::UnitTest::GetInstance()->current_test_info()->name() == std::string("RuntimeInitialization"))
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

// Basic Host Management Tests
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

// Null Handle Tests
TEST_F(NativeHostTest, NullHandleOperations)
{
    // Test create with null out_handle
    auto status = create(nullptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);

    // Test destroy with null handle
    status = destroy(nullptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);

    // Test initialize with null handle
    status = initialize(nullptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);

    // Test load with null handle
    native_assembly_handle_t assembly = nullptr;
    status = load(nullptr, "test.dll", &assembly);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);

    // Test unload with null handle
    status = unload(nullptr, assembly);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);

    // Test get_function_pointer with null handle
    void* fn_ptr = nullptr;
    status = get_function_pointer(nullptr, assembly, "type", "method", &fn_ptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);
}

// Invalid Handle Tests
TEST_F(NativeHostTest, InvalidHandleOperations)
{
    native_host_handle_t invalid_handle = reinterpret_cast<native_host_handle_t>(0xDEADBEEF);
    
    // Test operations with invalid handle
    auto status = initialize(invalid_handle);
    EXPECT_EQ(status, NativeHostStatus::ERROR_HOST_NOT_FOUND);

    native_assembly_handle_t assembly = nullptr;
    status = load(invalid_handle, "test.dll", &assembly);
    EXPECT_EQ(status, NativeHostStatus::ERROR_HOST_NOT_FOUND);

    status = unload(invalid_handle, assembly);
    EXPECT_EQ(status, NativeHostStatus::ERROR_HOST_NOT_FOUND);

    void* fn_ptr = nullptr;
    status = get_function_pointer(invalid_handle, assembly, "type", "method", &fn_ptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_HOST_NOT_FOUND);

    status = destroy(invalid_handle);
    EXPECT_EQ(status, NativeHostStatus::ERROR_HOST_NOT_FOUND);
}

// Assembly Management Tests
TEST_F(NativeHostTest, AssemblyLoadingAndUnloading)
{
    // Test loading with null path
    native_assembly_handle_t test_assembly = nullptr;
    auto status = load(host_handle_, nullptr, &test_assembly);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);

    // Test loading with null out handle
    status = load(host_handle_, assembly_path_.c_str(), nullptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);

    // Test loading non-existent assembly
    status = load(host_handle_, "nonexistent.dll", &test_assembly);
    EXPECT_NE(status, NativeHostStatus::SUCCESS);  // Just check it's not success

    // Test unloading null assembly handle
    status = unload(host_handle_, nullptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);

    // Test unloading invalid assembly handle
    native_assembly_handle_t invalid_assembly = reinterpret_cast<native_assembly_handle_t>(0xDEADBEEF);
    status = unload(host_handle_, invalid_assembly);
    EXPECT_EQ(status, NativeHostStatus::ERROR_ASSEMBLY_NOT_FOUND);
}

// Function Loading Tests
TEST_F(NativeHostTest, FunctionLoading)
{
    void* fn_ptr = nullptr;

    // Test with null type name
    auto status = get_function_pointer(host_handle_, assembly_handle_, nullptr, "method", &fn_ptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);

    // Test with null method name
    status = get_function_pointer(host_handle_, assembly_handle_, type_name_.c_str(), nullptr, &fn_ptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);

    // Test with null out function pointer
    status = get_function_pointer(host_handle_, assembly_handle_, type_name_.c_str(), "method", nullptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);

    // Test with empty type name
    status = get_function_pointer(host_handle_, assembly_handle_, "", "method", &fn_ptr);
    EXPECT_NE(status, NativeHostStatus::SUCCESS);

    // Test with empty method name
    status = get_function_pointer(host_handle_, assembly_handle_, type_name_.c_str(), "", &fn_ptr);
    EXPECT_NE(status, NativeHostStatus::SUCCESS);
}

// Runtime State Tests
TEST_F(NativeHostTest, RuntimeInitialization)
{
    // Create a new host
    native_host_handle_t new_handle = nullptr;
    auto status = create(&new_handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    EXPECT_NE(new_handle, nullptr);

    // Try to load assembly before initialization
    native_assembly_handle_t test_assembly = nullptr;
    status = load(new_handle, assembly_path_.c_str(), &test_assembly);
    EXPECT_EQ(status, NativeHostStatus::ERROR_ASSEMBLY_NOT_INITIALIZED);  // Should fail

    // Initialize runtime
    status = initialize(new_handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    // Now loading should succeed
    status = load(new_handle, assembly_path_.c_str(), &test_assembly);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    EXPECT_NE(test_assembly, nullptr);

    // Cleanup
    if (test_assembly != nullptr)
    {
        status = unload(new_handle, test_assembly);
        EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    }
    if (new_handle != nullptr)
    {
        status = destroy(new_handle);
        EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    }
}

// Concurrent Operation Tests
TEST_F(NativeHostTest, ConcurrentOperations)
{
    constexpr int NUM_THREADS = 4;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    // Ensure host is destroyed before concurrent test
    if (host_handle_ != nullptr)
    {
        if (assembly_handle_ != nullptr)
        {
            unload(host_handle_, assembly_handle_);
            assembly_handle_ = nullptr;
        }
        destroy(host_handle_);
        host_handle_ = nullptr;
    }

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads.emplace_back([&]() {
            native_host_handle_t thread_handle = nullptr;
            auto status = create(&thread_handle);
            if (status == NativeHostStatus::SUCCESS)
            {
                success_count++;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Add some delay
                destroy(thread_handle);
            }
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    // Only one thread should succeed in creating a host
    EXPECT_EQ(success_count.load(), 1);
}

// Function Call Tests
TEST_F(NativeHostTest, FunctionCallTests)
{
    auto add_fn = getFunctionPointer<AddNumbersDelegate>("AddNumbers");
    EXPECT_NE(add_fn, nullptr);

    // Test normal cases
    EXPECT_EQ(add_fn(0, 0), 0);
    EXPECT_EQ(add_fn(1, 1), 2);
    EXPECT_EQ(add_fn(-1, 1), 0);
    
    // Test boundary cases
    EXPECT_EQ(add_fn(INT32_MAX, 0), INT32_MAX);
    EXPECT_EQ(add_fn(0, INT32_MAX), INT32_MAX);
    EXPECT_EQ(add_fn(INT32_MIN, 0), INT32_MIN);
    EXPECT_EQ(add_fn(0, INT32_MIN), INT32_MIN);
}

// Multiple Assembly Tests
TEST_F(NativeHostTest, MultipleAssemblyOperations)
{
    constexpr int NUM_ASSEMBLIES = 5;
    std::vector<native_assembly_handle_t> assemblies;

    // Load multiple assemblies
    for (int i = 0; i < NUM_ASSEMBLIES; ++i)
    {
        native_assembly_handle_t assembly = nullptr;
        auto status = load(host_handle_, assembly_path_.c_str(), &assembly);
        EXPECT_EQ(status, NativeHostStatus::SUCCESS);
        assemblies.push_back(assembly);
    }

    // Test function calls from each assembly
    for (auto assembly : assemblies)
    {
        void* fn_ptr = nullptr;
        auto status = get_function_pointer(
            host_handle_,
            assembly,
            type_name_.c_str(),
            "ReturnConstant",
            &fn_ptr);
        EXPECT_EQ(status, NativeHostStatus::SUCCESS);
        
        auto fn = reinterpret_cast<ReturnConstantDelegate>(fn_ptr);
        EXPECT_EQ(fn(), 42);
    }

    // Unload all assemblies
    for (auto assembly : assemblies)
    {
        auto status = unload(host_handle_, assembly);
        EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    }
}