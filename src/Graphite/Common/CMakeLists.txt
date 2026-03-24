set(LIB_COMMON_SOURCES
    UniqueID.cpp
    # DynamicLibrary.cpp
)

add_library(graphite_core_common STATIC ${LIB_COMMON_SOURCES})

set_target_properties(graphite_core_common PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(graphite_core_common REUSE_FROM fluxion_pch)

target_link_libraries(graphite_core_common PRIVATE fluxion_compile_options)

target_include_directories(graphite_core_common PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/Graphite/Common
)
