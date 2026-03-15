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

target_include_directories(graphite_core_logger
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
