add_library(graphite_core_logger STATIC
    Logger.cpp
    Private/Ansi.cpp
    Private/LogFormatter.cpp
)

set_target_properties(graphite_core_logger PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# target_precompile_headers(graphite_core_logger REUSE_FROM fluxion_pch)

target_link_libraries(graphite_core_logger PRIVATE
    graphite_core_common
    fluxion_compile_options
)

target_include_directories(graphite_core_logger
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/external
)
