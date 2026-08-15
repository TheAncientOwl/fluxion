add_executable(test__fluxion_logs_plugin__text__regex_tags__v1
    PluginTest.cpp
)

target_link_libraries(test__fluxion_logs_plugin__text__regex_tags__v1 PUBLIC
    external_imgui

    fluxion_api_testing_interface
    fluxion_api_testing_impl

    fluxion_logs_plugin__text__regex_tags__v1_interface
    fluxion_logs_plugin__text__regex_tags__v1
)
