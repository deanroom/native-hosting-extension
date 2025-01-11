add_library(NativeHosting SHARED
    native_hosting.cpp
    native_hosting.h
)

target_include_directories(NativeHosting PRIVATE
    ${DOTNET_HOSTING_INCLUDE_PATH}
)

if(WIN32)
    target_compile_definitions(NativeHosting PRIVATE NATIVE_HOSTING_EXPORTS)
elseif(APPLE)
    target_compile_definitions(NativeHosting PRIVATE NATIVE_HOSTING_EXPORTS)
    set_target_properties(NativeHosting PROPERTIES
        MACOSX_RPATH ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

if(APPLE)
    target_link_libraries(NativeHosting PRIVATE "${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
    target_link_options(NativeHosting PRIVATE -framework CoreFoundation)
elseif(UNIX)
    target_link_libraries(NativeHosting PRIVATE "${DOTNET_HOSTING_LIB_PATH}/libnethost.a")
    target_link_libraries(NativeHosting PRIVATE dl)
endif() 