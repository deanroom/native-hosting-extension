#pragma once

/**
 * @file platform.h
 * @brief Platform-specific definitions and functions
 * 
 * This header provides platform-specific macros, types and functions
 * to handle differences between operating systems.
 */

#ifdef _WIN32
    #include <Windows.h>
    #define PLATFORM_WINDOWS 1
    #define MAX_PATH_LENGTH MAX_PATH
#else
    #include <dlfcn.h>
    #include <limits.h>
    #define PLATFORM_UNIX 1
    #define MAX_PATH_LENGTH PATH_MAX
#endif

#include <string>
#include <locale>
#include <codecvt>

namespace native_plugin_host {

#ifdef PLATFORM_WINDOWS
using char_t = wchar_t;
#else
using char_t = char;
#endif

/**
 * @brief Platform-specific library handle type
 */
#ifdef PLATFORM_WINDOWS
    using LibraryHandle = HMODULE;
#else
    using LibraryHandle = void*;
#endif

/**
 * @brief Convert a C-style string to platform-specific char_t string
 * @param str Input string
 * @return Platform-specific string
 */
inline std::basic_string<char_t> to_char_t(const char* str) {
#ifdef PLATFORM_WINDOWS
    std::wstring wstr = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
    return std::basic_string<char_t>(wstr.begin(), wstr.end());
#else
    return std::basic_string<char_t>(str);
#endif
}

/**
 * @brief Load a dynamic library
 * @param path Path to the library
 * @return Handle to the loaded library or nullptr on failure
 */
inline LibraryHandle load_library(const char* path) {
#ifdef PLATFORM_WINDOWS
    return LoadLibraryW(to_char_t(path).c_str());
#else
    return dlopen(path, RTLD_LAZY);
#endif
}

/**
 * @brief Get a function pointer from a loaded library
 * @param lib Handle to the loaded library
 * @param name Name of the function to get
 * @return Function pointer or nullptr on failure
 */
inline void* get_function(LibraryHandle lib, const char* name) {
#ifdef PLATFORM_WINDOWS
    return GetProcAddress(lib, name);
#else
    return dlsym(lib, name);
#endif
}

/**
 * @brief Unload a dynamic library
 * @param lib Handle to the loaded library
 * @return true if successful, false otherwise
 */
inline void free_library(LibraryHandle lib) {
#ifdef PLATFORM_WINDOWS
    ::FreeLibrary(lib);
#else
    dlclose(lib);
#endif
}

} // namespace native_plugin_host 