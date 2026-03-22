add_subdirectory(Renderer)

add_library(graphite_core_application INTERFACE)

set_target_properties(graphite_core_application PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# target_precompile_headers(graphite_core_application REUSE_FROM fluxion_pch)

target_sources(graphite_core_application
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            TGraphiteApplication.hpp
            ILayer.hpp
            WindowConfiguration.hpp
)

target_link_libraries(graphite_core_application INTERFACE
    fluxion_compile_options
    graphite_core_common
    graphite_core_logger
    graphite_core_renderer
)

target_include_directories(graphite_core_application INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
)
