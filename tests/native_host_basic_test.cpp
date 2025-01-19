#include <gtest/gtest.h>
#include "native_host.h"
#include "test_utils.h"

class NativeHostBasicTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(NativeHostBasicTest, CreateSucceeds)
{
    native_host_handle_t handle = nullptr;
    auto status = native_host_create(&handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    EXPECT_NE(handle, nullptr);

    status = native_host_destroy(handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
}

TEST_F(NativeHostBasicTest, CreateFailsWhenHostExists)
{
    native_host_handle_t first_handle = nullptr;
    auto status = native_host_create(&first_handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    native_host_handle_t second_handle = nullptr;
    status = native_host_create(&second_handle);
    EXPECT_EQ(status, NativeHostStatus::ERROR_HOST_ALREADY_EXISTS);

    status = native_host_destroy(first_handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
}

TEST_F(NativeHostBasicTest, DestroySucceeds)
{
    native_host_handle_t handle = nullptr;
    auto status = native_host_create(&handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    status = native_host_destroy(handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    // Can create again after destroy
    status = native_host_create(&handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    status = native_host_destroy(handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
}

TEST_F(NativeHostBasicTest, NullHandleOperations)
{
    EXPECT_EQ(native_host_create(nullptr), NativeHostStatus::ERROR_INVALID_ARG);
    EXPECT_EQ(native_host_destroy(nullptr), NativeHostStatus::ERROR_INVALID_ARG);
    EXPECT_EQ(native_host_initialize(nullptr), NativeHostStatus::ERROR_INVALID_ARG);
}

TEST_F(NativeHostBasicTest, InvalidHandleOperations)
{
    native_host_handle_t invalid_handle = reinterpret_cast<native_host_handle_t>(0xDEADBEEF);
    
    EXPECT_EQ(native_host_initialize(invalid_handle), NativeHostStatus::ERROR_HOST_NOT_FOUND);
    EXPECT_EQ(native_host_destroy(invalid_handle), NativeHostStatus::ERROR_HOST_NOT_FOUND);
}

TEST_F(NativeHostBasicTest, InitializationSucceeds)
{
    native_host_handle_t handle = nullptr;
    auto status = native_host_create(&handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    status = native_host_initialize(handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    status = native_host_destroy(handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
}

TEST_F(NativeHostBasicTest, MultipleInitializationIsSafe)
{
    native_host_handle_t handle = nullptr;
    auto status = native_host_create(&handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    status = native_host_initialize(handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    // Second initialization should succeed
    status = native_host_initialize(handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    status = native_host_destroy(handle);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
} 