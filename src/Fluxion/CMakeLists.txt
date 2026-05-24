add_subdirectory(API)
add_subdirectory(Data)
add_subdirectory(SentinelPlugins)

# --- 1. Implementation ---
add_library(fluxion_app STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Fluxion.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Views/Dev/DevView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/Dev/Modules/Logger.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Views/BaseView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/MainMenuView.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Views/Settings/SettingsView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/Settings/Modules/Theme.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/Settings/Modules/LogsPlugin.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Views/LogsTable/LogsTableView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/LogsTable/LogsTableViewActions.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Views/Filters/FiltersView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/Filters/FiltersViewActions.cpp
)

target_compile_definitions(fluxion_app PRIVATE GRAPHITE_LOGGER_BUILD)

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
    fluxion_sentinel_plugins_interface
    fluxion_sentinel_plugins
)

target_include_directories(fluxion_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
