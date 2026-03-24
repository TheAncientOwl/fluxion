add_library(fluxion_app STATIC
    Fluxion.cpp
    AppState.cpp

    Layers/DevLayer/DevLayer.cpp
    Layers/DevLayer/Modules/Logger.cpp
    Layers/DevLayer/Modules/Theme.cpp

    Layers/BaseLayer.cpp
    Layers/MainMenuLayer.cpp
    Layers/LogsViewLayer.cpp
    Layers/FiltersLayer.cpp
)

set_target_properties(fluxion_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(fluxion_app REUSE_FROM fluxion_pch)

target_link_libraries(fluxion_app PRIVATE
    imgui
    fluxion_compile_options
    fluxion_api
    graphite_core_logger
    graphite_core_renderer
    graphite_core_application
)
