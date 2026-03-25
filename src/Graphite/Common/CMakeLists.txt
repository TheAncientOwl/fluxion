# --- 1. Interface ---
add_library(graphite_common_interface INTERFACE)

target_include_directories(graphite_common_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- 2. Implementation ---
add_library(graphite_common_impl STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UniqueID.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/DynamicLibrary.cpp
)

set_target_properties(graphite_common_impl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(graphite_common_impl REUSE_FROM fluxion_pch)

target_link_libraries(graphite_common_impl PRIVATE
    project_compile_options
    graphite_common_interface
)

target_include_directories(graphite_common_impl PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)
