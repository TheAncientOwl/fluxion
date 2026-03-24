add_library(fluxion_api STATIC
    Data.cpp
)

set_target_properties(fluxion_api PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(fluxion_api REUSE_FROM fluxion_pch)

target_link_libraries(fluxion_api PRIVATE
    imgui
    fluxion_compile_options
    graphite_core_logger
    graphite_core_common
)

target_include_directories(fluxion_api PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/external
)
