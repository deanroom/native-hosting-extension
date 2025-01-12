#pragma once

/**
 * @file platform.h
 * @brief 平台特定的定义和函数
 *
 * 此头文件提供平台特定的宏、类型和函数，
 * 用于处理不同操作系统之间的差异。
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

namespace native_plugin_host
{

#ifdef PLATFORM_WINDOWS
    using char_t = wchar_t;
#else
    using char_t = char;
#endif

/**
 * @brief 平台特定的库句柄类型
 */
#ifdef PLATFORM_WINDOWS
    using LibraryHandle = HMODULE;
#else
    using LibraryHandle = void *;
#endif

    /**
     * @brief 将C风格字符串转换为平台特定的char_t字符串
     * @param str 输入字符串
     * @return 平台特定的字符串
     */
    inline std::basic_string<char_t> to_char_t(const char *str)
    {
#ifdef PLATFORM_WINDOWS
        std::wstring wstr = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
        return std::basic_string<char_t>(wstr.begin(), wstr.end());
#else
        return std::basic_string<char_t>(str);
#endif
    }

    /**
     * @brief 加载动态库
     * @param path 库的路径
     * @return 已加载库的句柄，失败时返回nullptr
     */
    inline LibraryHandle load_library(const char_t *path)
    {
#ifdef PLATFORM_WINDOWS
        return LoadLibraryW(path);
#else
        std::string narrow_path(path);
        return dlopen(narrow_path.c_str(), RTLD_LAZY);
#endif
    }

    /**
     * @brief 从已加载的库中获取函数指针
     * @param lib 已加载库的句柄
     * @param name 要获取的函数名称
     * @return 函数指针，失败时返回nullptr
     */
    inline void *get_function(LibraryHandle lib, const char *name)
    {
#ifdef PLATFORM_WINDOWS
        return GetProcAddress(lib, name);
#else
        return dlsym(lib, name);
#endif
    }

    /**
     * @brief 卸载动态库
     * @param lib 已加载库的句柄
     * @return 成功返回true，否则返回false
     */
    inline void free_library(LibraryHandle lib)
    {
#ifdef PLATFORM_WINDOWS
        ::FreeLibrary(lib);
#else
        dlclose(lib);
#endif
    }

} // namespace native_plugin_host
