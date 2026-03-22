add_library(fluxion_app STATIC
    Fluxion.cpp

    layers/DebugLayer/DebugLayer.cpp
    layers/DebugLayer/Modules/Logger.cpp

    layers/BaseLayer.cpp
    layers/MainMenuLayer.cpp
    layers/LogsViewLayer.cpp
    layers/FiltersLayer.cpp
)

set_target_properties(fluxion_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_link_libraries(fluxion_app PRIVATE
    fluxion_compile_options
    fluxion_api
    graphite_core_logger
    graphite_core_renderer
    graphite_core_application
)

target_include_directories(fluxion_app PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)
