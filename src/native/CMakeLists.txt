add_library(NativeHosting SHARED
    native_hosting.cpp
    native_hosting.h
)

target_include_directories(NativeHosting PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DOTNET_HOSTING_INCLUDE_PATH}
)

target_link_directories(NativeHosting PRIVATE
    ${DOTNET_HOSTING_LIB_PATH}
)

if(WIN32)
    target_compile_definitions(NativeHosting PRIVATE NATIVE_HOSTING_EXPORTS)
    target_link_libraries(NativeHosting PRIVATE libnethost)
elseif(APPLE)
    target_link_libraries(NativeHosting PRIVATE libnethost.dylib)
else()
    target_link_libraries(NativeHosting PRIVATE nethost)
endif() 