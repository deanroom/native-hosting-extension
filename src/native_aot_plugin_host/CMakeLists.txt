add_library(NativeAotPluginHost SHARED
    native_aot_plugin_host.cpp
    native_aot_plugin_host.h
)

target_include_directories(NativeAotPluginHost PRIVATE
    ${DOTNET_HOSTING_INCLUDE_PATH}
)

if(WIN32)
    target_compile_definitions(NativeAotPluginHost PRIVATE NATIVE_AOT_PLUGIN_HOST_EXPORTS)
elseif(APPLE)
    target_compile_definitions(NativeAotPluginHost PRIVATE NATIVE_AOT_PLUGIN_HOST_EXPORTS)
    set_target_properties(NativeAotPluginHost PROPERTIES
        MACOSX_RPATH ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

if(APPLE)
    target_link_libraries(NativeAotPluginHost PRIVATE "${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
    target_link_options(NativeAotPluginHost PRIVATE -framework CoreFoundation)
elseif(UNIX)
    target_link_libraries(NativeAotPluginHost PRIVATE "${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
    target_link_libraries(NativeAotPluginHost PRIVATE dl)
endif() 