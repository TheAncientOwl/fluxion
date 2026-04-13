# --- 1. Interface ---
add_library(fluxion_logs_plugin__regex_text_v1_interface INTERFACE)

target_include_directories(fluxion_logs_plugin__regex_text_v1_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/RegexTextV1/include
)

# --- 2. Implementation ---
add_library(fluxion_logs_plugin__regex_text_v1 SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/RegexTextV1/src/RegexTextV1LogsPlugin.cpp
)

set_target_properties(fluxion_logs_plugin__regex_text_v1 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
)

target_link_libraries(fluxion_logs_plugin__regex_text_v1 PRIVATE
    project_compile_options
    external_imgui
    external_icons_codicons
    graphite_common_interface
    graphite_common_impl
    graphite_logger_interface
    fluxion_api_interface
    fluxion_api_impl
    fluxion_logs_plugin__regex_text_v1_interface
)

target_include_directories(fluxion_logs_plugin__regex_text_v1 PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/Graphite/Logger/include
)

# --- 3. Install to ~/.fluxion/plugins/logs ---
add_custom_command(TARGET fluxion_logs_plugin__regex_text_v1 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $ENV{HOME}/.fluxion/plugins/logs
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:fluxion_logs_plugin__regex_text_v1> $ENV{HOME}/.fluxion/plugins/logs
    COMMENT "Copying logs plugin to ~/.fluxion/plugins/logs"
)
