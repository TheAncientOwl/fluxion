add_executable(test__fluxion_logs_plugin__text__regex_tags__v6
    PluginTest.cpp
)

target_link_libraries(test__fluxion_logs_plugin__text__regex_tags__v6 PUBLIC
    external_imgui

    fluxion_api_testing_interface

    fluxion_logs_plugin__text__regex_tags__v6_interface
    fluxion_logs_plugin__text__regex_tags__v6
)
