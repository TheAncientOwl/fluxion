add_library(fluxion_api SHARED
    Data.cpp
)

set_target_properties(fluxion_api PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_link_libraries(fluxion_api PRIVATE
    fluxion_compile_options
    graphite_core_logger
    graphite_core_common
)

target_include_directories(fluxion_api PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/external
)
