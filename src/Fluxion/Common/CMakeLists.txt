# --- 1. Interface ---
add_library(fluxion_common_interface INTERFACE)

target_include_directories(fluxion_common_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- 2. Implementation ---
add_library(fluxion_common_impl STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Math.cpp
)

set_target_properties(fluxion_common_impl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(fluxion_common_impl REUSE_FROM fluxion_pch)

target_link_libraries(fluxion_common_impl PRIVATE
    project_compile_options
    external_imgui
    external_icons_codicons
    fluxion_common_interface
    graphite_logger_interface
)

target_include_directories(fluxion_common_impl PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)
