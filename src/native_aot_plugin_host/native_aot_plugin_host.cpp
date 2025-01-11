#include "native_aot_plugin_host.h"
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <string>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#define PATH_SEPARATOR "\\"
#define MAX_PATH_LENGTH MAX_PATH

// Helper function to convert UTF-8 to wchar_t string (Windows)
std::wstring Utf8ToWide(const char* utf8Str)
{
    if (!utf8Str)
        return std::wstring();

    int requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, nullptr, 0);
    if (requiredSize <= 0)
    {
        return std::wstring();
    }

    std::wstring wideStr(requiredSize, L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &wideStr[0], requiredSize) == 0)
    {
        return std::wstring();
    }

    wideStr.resize(wcslen(wideStr.c_str()));
    return wideStr;
}

// Helper function to convert UTF-8 to char_t (wchar_t on Windows, char on Unix)
std::basic_string<char_t> ToCharT(const char* str)
{
#ifdef _WIN32
    return Utf8ToWide(str);
#else
    return std::string(str);
#endif
}

// Helper function to get Windows error message
std::string GetLastErrorAsString()
{
    DWORD error = GetLastError();
    if (error == 0)
    {
        return std::string();
    }

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer,
        0,
        NULL);

    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

// Helper function to convert UTF-8 to UTF-16
std::wstring Utf8ToUtf16(const char *utf8Str)
{
    if (!utf8Str)
        return std::wstring();

    int requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, nullptr, 0);
    if (requiredSize <= 0)
    {
        return std::wstring();
    }

    std::wstring utf16Str(requiredSize, L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &utf16Str[0], requiredSize) == 0)
    {
        return std::wstring();
    }

    return utf16Str;
}
#else
#include <dlfcn.h>
#include <limits.h>
#define PATH_SEPARATOR "/"
#define MAX_PATH_LENGTH PATH_MAX

// On Unix systems, char_t is char, so no conversion needed
std::string ToCharT(const char* str)
{
    return std::string(str);
}
#endif

// Globals for native hosting
static load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_ptr = nullptr;
static hostfxr_close_fn hostfxr_close_ptr = nullptr;

// Initialize the .NET runtime
bool initialize_runtime(const char* runtimeConfigPath)
{
    // Get hostfxr path
    char_t hostfxr_path[MAX_PATH_LENGTH];
    size_t buffer_size = sizeof(hostfxr_path) / sizeof(char_t);

    int rc = get_hostfxr_path(hostfxr_path, &buffer_size, nullptr);
    if (rc != 0)
    {
        std::cerr << "Failed to get hostfxr path, error code: " << rc << std::endl;
        return false;
    }

    // Load hostfxr library
#ifdef _WIN32
    void* lib = LoadLibraryW((LPCWSTR)hostfxr_path);
#else
    void* lib = dlopen(hostfxr_path, RTLD_LAZY | RTLD_LOCAL);
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
    auto config_path = ToCharT(runtimeConfigPath);
    rc = init_fptr(config_path.c_str(), nullptr, &cxt);

    if (rc > 1 || cxt == nullptr)
    {
        hostfxr_close_ptr(cxt);
        std::cerr << "Failed to initialize runtime. Error code: " << rc << std::endl;
        return false;
    }

    rc = get_delegate_fptr(
        cxt,
        hdt_load_assembly_and_get_function_pointer,
        (void**)&load_assembly_and_get_function_ptr);

    if (rc != 0 || load_assembly_and_get_function_ptr == nullptr)
    {
        hostfxr_close_ptr(cxt);
        std::cerr << "Failed to get load_assembly_and_get_function_ptr. Error code: " << rc << std::endl;
        return false;
    }

    hostfxr_close_ptr(cxt);
    return true;
}

// Load an assembly and get a function pointer
void* load_assembly_and_get_function_pointer(
    const char* assemblyPath,
    const char* typeName,
    const char* methodName,
    const char* delegateTypeName)
{
    if (load_assembly_and_get_function_ptr == nullptr)
    {
        std::cerr << "Runtime not initialized" << std::endl;
        return nullptr;
    }

    void* function_ptr = nullptr;

    auto assembly_path = ToCharT(assemblyPath);
    auto type_name = ToCharT(typeName);
    auto method_name = ToCharT(methodName);
    auto delegate_type_name = ToCharT(delegateTypeName);

    int rc = load_assembly_and_get_function_ptr(
        assembly_path.c_str(),
        type_name.c_str(),
        method_name.c_str(),
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr,
        &function_ptr);

    if (rc != 0 || function_ptr == nullptr)
    {
        std::cerr << "Failed to load assembly and get function pointer, errcode:" << rc << std::endl;
        return nullptr;
    }

    return function_ptr;
}

// Close the runtime and cleanup
void close_runtime()
{
    load_assembly_and_get_function_ptr = nullptr;
    hostfxr_close_ptr = nullptr;
}