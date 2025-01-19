#include <gtest/gtest.h>
#include "native_host.h"
#include "test_utils.h"
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

class NativeHostConcurrencyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        assembly_path_ = "../tests/TestLibrary.dll";
        type_name_ = "TestLibrary.TestClass,TestLibrary";
    }

    void TearDown() override {}

    std::string assembly_path_;
    std::string type_name_;
};

TEST_F(NativeHostConcurrencyTest, SingleHostMultipleThreads)
{
    native_host_handle_t host = nullptr;
    auto status = native_host_create(&host);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    status = native_host_initialize(host);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    constexpr int NUM_THREADS = 4;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads.emplace_back([&]()
                             {
            native_assembly_handle_t assembly = nullptr;
            auto status = native_host_load_assembly(host, assembly_path_.c_str(), &assembly);
            if (status == NativeHostStatus::SUCCESS)
            {
                success_count++;
                native_host_unload_assembly(host, assembly);
            } });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(success_count.load(), NUM_THREADS);
    native_host_destroy(host);
}

TEST_F(NativeHostConcurrencyTest, MultipleHostCreationAttempts)
{
    constexpr int NUM_THREADS = 4;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads.emplace_back([&]()
                             {
            native_host_handle_t thread_handle = nullptr;
            auto status = native_host_create(&thread_handle);
            if (status == NativeHostStatus::SUCCESS)
            {
                success_count++;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                native_host_destroy(thread_handle);
            } });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(success_count.load(), 1);
}

TEST_F(NativeHostConcurrencyTest, ConcurrentFunctionCalls)
{
    native_host_handle_t host = nullptr;
    auto status = native_host_create(&host);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    status = native_host_initialize(host);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    native_assembly_handle_t assembly = nullptr;
    status = native_host_load_assembly(host, assembly_path_.c_str(), &assembly);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    void *fn_ptr = nullptr;
    status = native_host_get_delegate(
        host,
        assembly,
        type_name_.c_str(),
        "AddNumbers",
        &fn_ptr);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    auto add_fn = reinterpret_cast<int32_t (*)(int32_t, int32_t)>(fn_ptr);

    constexpr int NUM_THREADS = 4;
    std::vector<std::thread> threads;
    std::atomic<int> error_count{0};

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads.emplace_back([&, i]()
                             {
            for (int j = 0; j < 1000; ++j)
            {
                if (add_fn(i, j) != i + j)
                {
                    error_count++;
                }
            } });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(error_count.load(), 0);

    native_host_unload_assembly(host, assembly);
    native_host_destroy(host);
}

TEST_F(NativeHostConcurrencyTest, ConcurrentAssemblyOperations)
{
    native_host_handle_t host = nullptr;
    auto status = native_host_create(&host);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    status = native_host_initialize(host);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    constexpr int NUM_THREADS = 4;
    constexpr int OPERATIONS_PER_THREAD = 100;
    std::vector<std::thread> threads;
    std::atomic<int> error_count{0};

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads.emplace_back([&]()
                             {
            for (int j = 0; j < OPERATIONS_PER_THREAD; ++j)
            {
                native_assembly_handle_t assembly = nullptr;
                auto status = native_host_load_assembly(host, assembly_path_.c_str(), &assembly);
                if (status != NativeHostStatus::SUCCESS)
                {
                    error_count++;
                    continue;
                }

                void* fn_ptr = nullptr;
                status =  native_host_get_delegate(
                    host,
                    assembly,
                    type_name_.c_str(),
                    "ReturnConstant",
                    &fn_ptr);
                if (status != NativeHostStatus::SUCCESS)
                {
                    error_count++;
                }
                else
                {
                    auto fn = reinterpret_cast<int32_t(*)()>(fn_ptr);
                    if (fn() != 42)
                    {
                        error_count++;
                    }
                }

                status = native_host_unload_assembly(host, assembly);
                if (status != NativeHostStatus::SUCCESS)
                {
                    error_count++;
                }
            } });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(error_count.load(), 0);
    native_host_destroy(host);
}