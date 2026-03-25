# --- 1. Interface ---
add_library(fluxion_api_interface INTERFACE)

target_include_directories(fluxion_api_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# -- 2. Implementation ---
add_library(fluxion_api_impl STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Data.cpp
)

set_target_properties(fluxion_api_impl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(fluxion_api_impl REUSE_FROM fluxion_pch)

target_link_libraries(fluxion_api_impl PRIVATE
    external_imgui
    project_compile_options
    graphite_logger_interface
    graphite_common_interface
    fluxion_api_interface
)
