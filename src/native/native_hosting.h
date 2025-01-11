#pragma once

#ifdef _WIN32
#ifdef NATIVE_HOSTING_EXPORTS
#define NATIVE_HOSTING_API __declspec(dllexport)
#else
#define NATIVE_HOSTING_API __declspec(dllimport)
#endif
#else
#ifdef NATIVE_HOSTING_EXPORTS
#define NATIVE_HOSTING_API __attribute__((visibility("default")))
#else
#define NATIVE_HOSTING_API
#endif
#endif

#include <string>

extern "C"
{
    // Initialize the .NET runtime
    NATIVE_HOSTING_API bool initialize_runtime(const char *runtimeConfigPath);

    // Load an assembly and get a function pointer
    NATIVE_HOSTING_API void *load_assembly_and_get_function_pointer(
        const char *assemblyPath,
        const char *typeName,
        const char *methodName,
        const char *delegateTypeName);
        
    // Close the runtime and cleanup
    NATIVE_HOSTING_API void close_runtime();
}