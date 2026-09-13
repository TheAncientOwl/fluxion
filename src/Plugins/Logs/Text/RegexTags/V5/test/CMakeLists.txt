file(GLOB PLUGIN_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/SQLite/Wrapper/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/SQLite/*.cpp"
)

add_executable(test__fluxion_logs_plugin__text__regex_tags__v5
    ${PLUGIN_SOURCES}
    PluginTest.cpp
)

target_link_libraries(test__fluxion_logs_plugin__text__regex_tags__v5 PUBLIC
    external_imgui

    fluxion_api_testing_interface

    fluxion_logs_plugin__text__regex_tags__v5_interface
    fluxion_logs_plugin__text__regex_tags__v5
)
