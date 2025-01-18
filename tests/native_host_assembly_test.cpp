#include <gtest/gtest.h>
#include "native_host.h"
#include "test_utils.h"

class NativeHostAssemblyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        assembly_path_ = "../tests/TestLibrary.dll";
        type_name_ = "TestLibrary.TestClass,TestLibrary";

        status_ = create(&host_handle_);
        EXPECT_EQ(status_, NativeHostStatus::SUCCESS);
        EXPECT_NE(host_handle_, nullptr);

        status_ = initialize(host_handle_);
        EXPECT_EQ(status_, NativeHostStatus::SUCCESS);
    }

    void TearDown() override
    {
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

    native_host_handle_t host_handle_ = nullptr;
    native_assembly_handle_t assembly_handle_ = nullptr;
    NativeHostStatus status_ = NativeHostStatus::SUCCESS;
    std::string assembly_path_;
    std::string type_name_;
};

TEST_F(NativeHostAssemblyTest, LoadSucceeds)
{
    native_assembly_handle_t assembly = nullptr;
    auto status = load(host_handle_, assembly_path_.c_str(), &assembly);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    EXPECT_NE(assembly, nullptr);

    status = unload(host_handle_, assembly);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
}

TEST_F(NativeHostAssemblyTest, LoadFailsWithNullPath)
{
    native_assembly_handle_t assembly = nullptr;
    auto status = load(host_handle_, nullptr, &assembly);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);
}

TEST_F(NativeHostAssemblyTest, LoadFailsWithNullHandle)
{
    auto status = load(host_handle_, assembly_path_.c_str(), nullptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);
}

TEST_F(NativeHostAssemblyTest, LoadFailsWithNonexistentAssembly)
{
    native_assembly_handle_t assembly = nullptr;
    auto status = load(host_handle_, "nonexistent.dll", &assembly);
    EXPECT_EQ(status, NativeHostStatus::ERROR_ASSEMBLY_LOAD);
}

TEST_F(NativeHostAssemblyTest, UnloadSucceeds)
{
    native_assembly_handle_t assembly = nullptr;
    auto status = load(host_handle_, assembly_path_.c_str(), &assembly);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);

    status = unload(host_handle_, assembly);
    EXPECT_EQ(status, NativeHostStatus::SUCCESS);
}

TEST_F(NativeHostAssemblyTest, UnloadFailsWithNullHandle)
{
    auto status = unload(host_handle_, nullptr);
    EXPECT_EQ(status, NativeHostStatus::ERROR_INVALID_ARG);
}

TEST_F(NativeHostAssemblyTest, UnloadFailsWithInvalidHandle)
{
    native_assembly_handle_t invalid_assembly = reinterpret_cast<native_assembly_handle_t>(0xDEADBEEF);
    auto status = unload(host_handle_, invalid_assembly);
    EXPECT_EQ(status, NativeHostStatus::ERROR_ASSEMBLY_NOT_FOUND);
}

TEST_F(NativeHostAssemblyTest, MultipleAssemblyLoading)
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

    // Unload all assemblies
    for (auto assembly : assemblies)
    {
        auto status = unload(host_handle_, assembly);
        EXPECT_EQ(status, NativeHostStatus::SUCCESS);
    }
} 