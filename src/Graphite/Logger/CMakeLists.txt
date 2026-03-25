# --- 1. Interface ---
add_library(graphite_logger_interface INTERFACE)

target_include_directories(graphite_logger_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- 2. Implementation ---
add_library(graphite_logger_impl STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Ansi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LogFormatter.cpp
)

set_target_properties(graphite_logger_impl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(graphite_logger_impl REUSE_FROM fluxion_pch)

target_link_libraries(graphite_logger_impl PRIVATE
    external_imgui
    project_compile_options
    external_icons_codicons
    graphite_logger_interface
    graphite_common_interface
)

target_include_directories(graphite_logger_impl
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/external
)
