#include "native_hosting.h"
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <string>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#define PATH_SEPARATOR "\\"
#define MAX_PATH_LENGTH MAX_PATH
#else
#include <dlfcn.h>
#include <limits.h>
#define PATH_SEPARATOR "/"
#define MAX_PATH_LENGTH PATH_MAX
#endif

// Globals for native hosting
static load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_ptr = nullptr;
static hostfxr_close_fn hostfxr_close_ptr = nullptr;

// Initialize the .NET runtime
bool initialize_runtime(const char *runtimeConfigPath)
{
    // Get hostfxr path
    char_t hostfxr_path[MAX_PATH_LENGTH];
    size_t buffer_size = sizeof(hostfxr_path) / sizeof(char_t);

    int rc = get_hostfxr_path(hostfxr_path, &buffer_size, nullptr);
    if (rc != 0)
    {
        std::cerr << "Failed to get hostfxr path" << std::endl;
        return false;
    }

    // Load hostfxr library
#ifdef _WIN32
    void *lib = LoadLibraryW(hostfxr_path);
#else
    void *lib = dlopen(hostfxr_path, RTLD_LAZY | RTLD_LOCAL);
#endif
    if (lib == nullptr)
    {
        std::cerr << "Failed to load hostfxr" << std::endl;
        return false;
    }

    // Get required functions
    auto init_fptr = (hostfxr_initialize_for_runtime_config_fn)
#ifdef _WIN32
        GetProcAddress((HMODULE)lib, "hostfxr_initialize_for_runtime_config");
#else
        dlsym(lib, "hostfxr_initialize_for_runtime_config");
#endif

    auto get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)
#ifdef _WIN32
        GetProcAddress((HMODULE)lib, "hostfxr_get_runtime_delegate");
#else
        dlsym(lib, "hostfxr_get_runtime_delegate");
#endif

    hostfxr_close_ptr = (hostfxr_close_fn)
#ifdef _WIN32
        GetProcAddress((HMODULE)lib, "hostfxr_close");
#else
        dlsym(lib, "hostfxr_close");
#endif

    if (!init_fptr || !get_delegate_fptr || !hostfxr_close_ptr)
    {
        std::cerr << "Failed to get hostfxr function pointers" << std::endl;
        return false;
    }

    hostfxr_handle cxt = nullptr;
    // Initialize runtime
    // hostfxr_initialize_for_runtime_config_fn has the Return value:
    //    Success                            - Hosting components were successfully initialized
    //    Success_HostAlreadyInitialized     - Config is compatible with already initialized hosting components
    //    Success_DifferentRuntimeProperties - Config has runtime properties that differ from already initialized hosting components
    //    CoreHostIncompatibleConfig         - Config is incompatible with already initialized hosting components
    //  we treat rc > 1 as failed, because we except the runtime will has the same config as the one we initialized
    rc = init_fptr(runtimeConfigPath, nullptr, &cxt);
    if (rc > 1 || cxt == nullptr)
    {
        hostfxr_close_ptr(cxt);
        std::cerr << "Failed to initialize runtime" << std::endl;
        return false;
    }

    // Get function pointer for loading assemblies
    rc = get_delegate_fptr(
        cxt,
        hdt_load_assembly_and_get_function_pointer,
        (void **)&load_assembly_and_get_function_ptr);

    if (rc != 0 || load_assembly_and_get_function_ptr == nullptr)
    {
        hostfxr_close_ptr(cxt);
        std::cerr << "Failed to get load_assembly_and_get_function_ptr" << std::endl;
        return false;
    }

    hostfxr_close_ptr(cxt);
    return true;
}

// Load an assembly and get a function pointer
void *load_assembly_and_get_function_pointer(
    const char *assemblyPath,
    const char *typeName,
    const char *methodName,
    const char *delegateTypeName)
{

    if (load_assembly_and_get_function_ptr == nullptr)
    {
        std::cerr << "Runtime not initialized" << std::endl;
        return nullptr;
    }

    void *function_ptr = nullptr;

    int rc = load_assembly_and_get_function_ptr(
        assemblyPath,
        typeName,
        methodName,
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr,
        &function_ptr);

    if (rc != 0 || function_ptr == nullptr)
    {
        std::cerr << "Failed to load assembly and get function pointer,errcode:" << rc << std::endl;
        return nullptr;
    }

    return function_ptr;
}

// Close the runtime and cleanup, in fact it can not really close the runtime,
// because native hosting does not support it, just reset the function pointers
void close_runtime()
{
    load_assembly_and_get_function_ptr = nullptr;
    hostfxr_close_ptr = nullptr;
}