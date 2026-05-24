# --- 1. Interface ---
add_library(fluxion_sentinel_plugin__logs_interface INTERFACE)

target_include_directories(fluxion_sentinel_plugin__logs_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- 2. Implementation ---
add_library(fluxion_sentinel_plugin__logs STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SentinelLogsPlugin.cpp
)

set_target_properties(fluxion_sentinel_plugin__logs PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
)

target_link_libraries(fluxion_sentinel_plugin__logs PUBLIC
    project_compile_options
    graphite_interface
    graphite_impl
    fluxion_api_interface
    fluxion_api_impl
    fluxion_sentinel_plugin__logs_interface
)

target_include_directories(fluxion_sentinel_plugin__logs PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/external/imgui
)
