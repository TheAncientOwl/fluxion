set(LIB_COMMON_SOURCES
    UniqueID.cpp
    # DynamicLibrary.cpp
)

add_library(graphite_core_common SHARED ${LIB_COMMON_SOURCES})

set_target_properties(graphite_core_common PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_include_directories(graphite_core_common PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/Core/Common
)
