add_subdirectory(Data)

# --- 1. Implementation ---
add_library(fluxion_app STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Fluxion.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Layers/DevLayer/DevLayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Layers/DevLayer/Modules/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Layers/DevLayer/Modules/Theme.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Layers/BaseLayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Layers/MainMenuLayer.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Layers/SettingsLayer/SettingsLayer.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Layers/LogsViewLayer/LogsViewLayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Layers/LogsViewLayer/LogsViewLayerActions.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Layers/FiltersLayer/FiltersLayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Layers/FiltersLayer/FiltersLayerActions.cpp
)

set_target_properties(fluxion_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(fluxion_app REUSE_FROM fluxion_pch)

target_link_libraries(fluxion_app PUBLIC
    external_imgui
    external_icons_codicons
    project_compile_options
    graphite_interface
    graphite_impl
    fluxion_api_interface
    fluxion_api_impl
    fluxion_application_data_interface
    fluxion_application_data_impl
)

target_include_directories(fluxion_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
