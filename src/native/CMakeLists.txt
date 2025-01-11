add_library(NativeHosting SHARED
    native_hosting.cpp
    native_hosting.h
)

target_include_directories(NativeHosting PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(WIN32)
    target_compile_definitions(NativeHosting PRIVATE NATIVE_HOSTING_EXPORTS)
endif() 