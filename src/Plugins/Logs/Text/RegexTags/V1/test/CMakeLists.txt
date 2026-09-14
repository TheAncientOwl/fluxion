file(GLOB PLUGIN_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/CSV/Wrapper/*.cpp"
)

add_executable(test__fluxion_logs_plugin__text__regex_tags__v1
    ${PLUGIN_SOURCES}
    PluginTest.cpp
)

target_link_libraries(test__fluxion_logs_plugin__text__regex_tags__v1 PUBLIC
    external_imgui

    fluxion_api_testing_interface

    fluxion_logs_plugin__text__regex_tags__v1_interface
    fluxion_logs_plugin__text__regex_tags__v1
)

set_target_properties(test__fluxion_logs_plugin__text__regex_tags__v1 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    add_custom_command(TARGET test__fluxion_logs_plugin__text__regex_tags__v1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/bin/plugins/logs/$<TARGET_FILE_NAME:fluxion_logs_plugin__text__regex_tags__v1>"
            $<TARGET_FILE_DIR:test__fluxion_logs_plugin__text__regex_tags__v1>
        COMMENT "Copying V1 plugin DLL from plugins/logs to test binary directory"
    )
endif()

include(GoogleTest)
gtest_discover_tests(test__fluxion_logs_plugin__text__regex_tags__v1
    DISCOVERY_MODE PRE_TEST
)
