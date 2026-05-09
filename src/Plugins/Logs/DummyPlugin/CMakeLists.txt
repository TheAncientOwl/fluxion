# --- 1. Interface ---
add_library(fluxion_logs_plugin__dummy_plugin_interface INTERFACE)

target_include_directories(fluxion_logs_plugin__dummy_plugin_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- 2. Implementation ---
add_library(fluxion_logs_plugin__dummy_plugin SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DummyLogsPlugin.cpp
)

set_target_properties(fluxion_logs_plugin__dummy_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
)

target_link_libraries(fluxion_logs_plugin__dummy_plugin PUBLIC
    project_compile_options
    external_icons_codicons
    external_miocsv
    graphite_interface
    graphite_impl
    fluxion_api_interface
    fluxion_api_impl
    fluxion_logs_plugin__dummy_plugin_interface
)

target_include_directories(fluxion_logs_plugin__dummy_plugin PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/Graphite/Logger/include
    ${PROJECT_SOURCE_DIR}/external/imgui
)

# --- 3. Install to ~/.fluxion/plugins/logs ---
add_custom_command(TARGET fluxion_logs_plugin__dummy_plugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $ENV{HOME}/.fluxion/plugins/logs
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:fluxion_logs_plugin__dummy_plugin> $ENV{HOME}/.fluxion/plugins/logs
    COMMENT "Copying logs plugin to ~/.fluxion/plugins/logs"
)

# Also install on every cmake configure to ensure plugin is available even if not rebuilt
add_custom_target(install_dummy_plugin_plugin ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory $ENV{HOME}/.fluxion/plugins/logs
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:fluxion_logs_plugin__dummy_plugin> $ENV{HOME}/.fluxion/plugins/logs
    DEPENDS fluxion_logs_plugin__dummy_plugin
    COMMENT "Installing logs plugin to ~/.fluxion/plugins/logs"
)
