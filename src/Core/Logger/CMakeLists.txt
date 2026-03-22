add_library(graphite_core_logger SHARED
    Logger.cpp
    Logger.hpp
    Ansi.cpp
    Ansi.hpp
    LogFormatter.cpp
    LogFormatter.hpp
)

set_target_properties(graphite_core_logger PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_link_libraries(graphite_core_logger PRIVATE
    graphite_core_common
    fluxion_compile_options
)

target_include_directories(graphite_core_logger
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/external
)
