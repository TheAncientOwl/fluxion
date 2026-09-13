file(GLOB PLUGIN_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/CSV/Wrapper/*.cpp"
)

add_executable(test__fluxion_logs_plugin__text__regex_tags__v2
    ${PLUGIN_SOURCES}
    PluginTest.cpp
)

target_link_libraries(test__fluxion_logs_plugin__text__regex_tags__v2 PUBLIC
    external_imgui

    fluxion_api_testing_interface

    fluxion_logs_plugin__text__regex_tags__v2_interface
    fluxion_logs_plugin__text__regex_tags__v2
)
