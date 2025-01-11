add_library(native_aot_plugin_host SHARED
    native_aot_plugin_host.cpp
    native_aot_plugin_host.h
)

target_include_directories(native_aot_plugin_host PRIVATE
    ${DOTNET_HOSTING_INCLUDE_PATH}
)

if(WIN32)
    target_compile_definitions(native_aot_plugin_host PRIVATE NATIVE_AOT_PLUGIN_HOST_EXPORTS)
elseif(APPLE)
    target_compile_definitions(native_aot_plugin_host PRIVATE NATIVE_AOT_PLUGIN_HOST_EXPORTS)
    set_target_properties(native_aot_plugin_host PROPERTIES
        MACOSX_RPATH ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

if(APPLE)
    target_link_libraries(native_aot_plugin_host PRIVATE "${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
    target_link_options(native_aot_plugin_host PRIVATE -framework CoreFoundation)
elseif(UNIX)
    target_link_libraries(native_aot_plugin_host PRIVATE "${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
    target_link_libraries(native_aot_plugin_host PRIVATE dl)
endif() 