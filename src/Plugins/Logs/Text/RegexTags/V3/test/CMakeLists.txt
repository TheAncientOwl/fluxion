add_executable(test__fluxion_logs_plugin__text__regex_tags__v3
    PluginTest.cpp
)

target_link_libraries(test__fluxion_logs_plugin__text__regex_tags__v3 PUBLIC
    external_imgui

    fluxion_api_testing_interface

    fluxion_logs_plugin__text__regex_tags__v3_interface
    fluxion_logs_plugin__text__regex_tags__v3
)
