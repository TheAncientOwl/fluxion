# --- 1. Interface ---
add_library(fluxion_application_data_interface INTERFACE)

target_include_directories(fluxion_application_data_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(fluxion_application_data_interface PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(graphite_application_interface REUSE_FROM fluxion_pch)

target_link_libraries(fluxion_application_data_interface INTERFACE
    project_compile_options
    graphite_common_interface
    graphite_logger_interface
    graphite_renderer_interface
)

# --- 2. Implementation ---
add_library(fluxion_application_data_impl STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AppState.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Data.cpp
)

set_target_properties(fluxion_application_data_impl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(fluxion_application_data_impl REUSE_FROM fluxion_pch)

target_link_libraries(fluxion_application_data_impl PRIVATE
    external_imgui
    project_compile_options
    external_icons_codicons
    graphite_logger_interface
    graphite_common_interface
    fluxion_api_interface
    fluxion_application_data_interface
)

target_include_directories(fluxion_application_data_impl
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/external
)
