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

set_target_properties(test__fluxion_logs_plugin__text__regex_tags__v5 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    add_custom_command(TARGET test__fluxion_logs_plugin__text__regex_tags__v5 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/bin/plugins/logs/$<TARGET_FILE_NAME:fluxion_logs_plugin__text__regex_tags__v5>"
            $<TARGET_FILE_DIR:test__fluxion_logs_plugin__text__regex_tags__v5>
        COMMENT "Copying V5 plugin DLL from plugins/logs to test binary directory"
    )
endif()

include(GoogleTest)
gtest_discover_tests(test__fluxion_logs_plugin__text__regex_tags__v5
    DISCOVERY_MODE PRE_TEST
)
