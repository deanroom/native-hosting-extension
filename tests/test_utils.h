#pragma once

#include <string>
#include <filesystem>

// Common test utilities
namespace test_utils
{
    // Get the path to the test data directory
    inline std::string get_test_data_path()
    {
        return "../tests";
    }

    // Get the path to a test assembly
    inline std::string get_test_assembly_path(const std::string& assembly_name)
    {
        return (std::filesystem::path(get_test_data_path()) / assembly_name).string();
    }

    // Get the full type name for a test class
    inline std::string get_test_type_name(const std::string& class_name, const std::string& assembly_name)
    {
        return class_name + "," + assembly_name;
    }
} 