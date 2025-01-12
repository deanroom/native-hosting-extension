add_library(native_plugin_host SHARED
    native_plugin_host.cpp
    native_plugin_host.h
)

target_include_directories(native_plugin_host PRIVATE
    ${DOTNET_HOSTING_INCLUDE_PATH}
)

# Add compile definitions for all platforms
target_compile_definitions(native_plugin_host PRIVATE 
    NATIVE_PLUGIN_HOST_EXPORTS
)

if(WIN32)
    message(STATUS "Configuring Windows build:")
    message(STATUS "  System Processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
    message(STATUS "  Host Architecture: ${HOST_ARCH}")
    message(STATUS "  Library Path: ${DOTNET_HOSTING_LIB_PATH}")

    # Use static runtime library to match nethost.lib
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:>")
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()

    target_compile_definitions(native_plugin_host PRIVATE 
        NATIVE_PLUGIN_HOST_EXPORTS
        _WINDOWS
        _USRDLL
        NETHOST_USE_AS_STATIC
    )
    
    set_target_properties(native_plugin_host PROPERTIES
        PREFIX ""
        SUFFIX ".dll"
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:>"
    )

    # Check for nethost.dll and copy it
    if(NOT EXISTS "${DOTNET_HOSTING_LIB_PATH}/nethost.dll")
        message(FATAL_ERROR "nethost.dll not found at ${DOTNET_HOSTING_LIB_PATH}/nethost.dll")
    endif()
    file(COPY "${DOTNET_HOSTING_LIB_PATH}/nethost.dll" 
         DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    message(STATUS "Copied nethost.dll to output directory")

    # List all files in the library path for debugging
    message(STATUS "Files in library path:")
    file(GLOB LIB_FILES "${DOTNET_HOSTING_LIB_PATH}/*")
    foreach(FILE ${LIB_FILES})
        message(STATUS "  ${FILE}")
    endforeach()

    # Check and link against appropriate library
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64")
        if(NOT EXISTS "${DOTNET_HOSTING_LIB_PATH}/nethost.lib")
            message(FATAL_ERROR "nethost.lib not found at ${DOTNET_HOSTING_LIB_PATH}/nethost.lib")
        endif()
        target_link_libraries(native_plugin_host PRIVATE 
            "${DOTNET_HOSTING_LIB_PATH}/nethost.lib"
            shlwapi.lib
            pathcch.lib
            version.lib
        )
        message(STATUS "Using x64 nethost.lib")
    else()
        if(NOT EXISTS "${DOTNET_HOSTING_LIB_PATH}/libnethost.lib")
            message(FATAL_ERROR "libnethost.lib not found at ${DOTNET_HOSTING_LIB_PATH}/libnethost.lib")
        endif()
        target_link_libraries(native_plugin_host PRIVATE 
            "${DOTNET_HOSTING_LIB_PATH}/libnethost.lib"
            shlwapi.lib
            pathcch.lib
            version.lib
        )
        message(STATUS "Using ARM64 libnethost.lib")
    endif()

elseif(APPLE)
    message(STATUS "Configuring macOS build:")
    message(STATUS "  System Processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
    message(STATUS "  Host Architecture: ${HOST_ARCH}")
    message(STATUS "  Library Path: ${DOTNET_HOSTING_LIB_PATH}")

    target_compile_definitions(native_plugin_host PRIVATE 
        native_plugin_host_EXPORTS
    )
    
    set_target_properties(native_plugin_host PROPERTIES
        MACOSX_RPATH ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        PREFIX "lib"
        SUFFIX ".dylib"
    )

    # Check for libnethost.a
    if(NOT EXISTS "${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
        message(FATAL_ERROR "libnethost.a not found at ${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
    endif()
    
    # List all files in the library path for debugging
    message(STATUS "Files in library path:")
    file(GLOB LIB_FILES "${DOTNET_HOSTING_LIB_PATH}/*")
    foreach(FILE ${LIB_FILES})
        message(STATUS "  ${FILE}")
    endforeach()

    target_link_libraries(native_plugin_host PRIVATE "${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
    target_link_options(native_plugin_host PRIVATE -framework CoreFoundation)
else()
    target_compile_definitions(native_plugin_host PRIVATE native_plugin_host_EXPORTS)
    set_target_properties(native_plugin_host PROPERTIES
        PREFIX "lib"
        SUFFIX ".so"
    )
    target_link_libraries(native_plugin_host PRIVATE "${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
    target_link_libraries(native_plugin_host PRIVATE dl)
endif() 