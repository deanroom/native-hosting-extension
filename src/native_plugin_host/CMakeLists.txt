set(SOURCES
    native_plugin_host.cpp
)

set(HEADERS
    native_plugin_host.h
    platform.h
)

add_library(native_plugin_host SHARED ${SOURCES} ${HEADERS})

target_compile_definitions(native_plugin_host PRIVATE NATIVE_PLUGIN_HOST_EXPORTS)

# Include directories
target_include_directories(native_plugin_host PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${DOTNET_INCLUDE_PATH}
)

# Link libraries
target_link_libraries(native_plugin_host PRIVATE
    ${DOTNET_LIBRARY_PATH}/libnethost${CMAKE_STATIC_LIBRARY_SUFFIX}
)

target_link_libraries(native_plugin_host PRIVATE
    ${DOTNET_LIBRARY_PATH}/libnethost${CMAKE_SHARED_LIBRARY_SUFFIX}
)

# Set output directory
set_target_properties(native_plugin_host PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
) 