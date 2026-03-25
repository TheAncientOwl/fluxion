# --- 1. Interface ---
add_library(graphite_application_interface INTERFACE)

target_include_directories(graphite_application_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(graphite_application_interface PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(graphite_application_interface REUSE_FROM fluxion_pch)

target_link_libraries(graphite_application_interface INTERFACE
    project_compile_options
    graphite_common_interface
    graphite_logger_interface
    graphite_renderer_interface
)
