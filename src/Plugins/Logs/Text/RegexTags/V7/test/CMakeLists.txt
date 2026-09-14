# --- Plugin Tests ---
add_executable(test__fluxion_logs_plugin__text__regex_tags__v7
    PluginTest.cpp
)

target_link_libraries(test__fluxion_logs_plugin__text__regex_tags__v7 PUBLIC
    external_imgui

    fluxion_api_testing_interface

    fluxion_logs_plugin__text__regex_tags__v7_interface
    fluxion_logs_plugin__text__regex_tags__v7
)

# --- Unit Tests ---
include(FetchContent)
FetchContent_Declare(
  googletest
  SYSTEM
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.18.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(unit_tests__fluxion_logs_plugin__text__regex_tags__v7
    MappedBinaryStringsFileTest.cpp
    PapyrusTest.cpp
    ScribeTest.cpp

    ../src/Scrolls/MappedBinaryStringsFile.cpp
    ../src/Scrolls/Papyrus.cpp
    ../src/Scrolls/Scribe.cpp
)

target_include_directories(unit_tests__fluxion_logs_plugin__text__regex_tags__v7 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${PROJECT_SOURCE_DIR}/src/Graphite/Logger/include
    ${PROJECT_SOURCE_DIR}/external/imgui
)

target_link_libraries(unit_tests__fluxion_logs_plugin__text__regex_tags__v7 PRIVATE
    gtest_main
    fluxion_logs_plugin__text__regex_tags__v7
)

# Fix: Apply the bin directory to BOTH executables
set_target_properties(
    test__fluxion_logs_plugin__text__regex_tags__v7 
    unit_tests__fluxion_logs_plugin__text__regex_tags__v7
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    add_custom_command(TARGET test__fluxion_logs_plugin__text__regex_tags__v7 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/bin/plugins/logs/$<TARGET_FILE_NAME:fluxion_logs_plugin__text__regex_tags__v7>"
            $<TARGET_FILE_DIR:test__fluxion_logs_plugin__text__regex_tags__v7>
        COMMENT "Copying V7 plugin DLL from plugins/logs to test binary directory"
    )
    # Note: Since we pushed both executables to the exact same 'bin' directory, 
    # this one copy command is enough to fix the DLL issue for BOTH of them!
endif()

include(GoogleTest)
gtest_discover_tests(test__fluxion_logs_plugin__text__regex_tags__v7
    DISCOVERY_MODE PRE_TEST
)
gtest_discover_tests(unit_tests__fluxion_logs_plugin__text__regex_tags__v7
    DISCOVERY_MODE PRE_TEST
)
