file(GLOB PLUGIN_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/SQLite/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/Data.cpp"
)

# --- Plugin Tests ---
add_executable(test__fluxion_logs_plugin__text__regex_tags__v8
    ${PLUGIN_SOURCES}
    PluginTest.cpp
)

target_link_libraries(test__fluxion_logs_plugin__text__regex_tags__v8 PUBLIC
    external_imgui

    fluxion_api_testing_interface

    fluxion_logs_plugin__text__regex_tags__v8_interface
    fluxion_logs_plugin__text__regex_tags__v8
)

# --- SQLiteStorage Unit Tests ---
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

add_executable(unit_tests__fluxion_logs_plugin__text__regex_tags__v8_sqlite_storage
    ${PLUGIN_SOURCES}
    SQLiteStorageTest.cpp
)

target_include_directories(unit_tests__fluxion_logs_plugin__text__regex_tags__v8_sqlite_storage PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(unit_tests__fluxion_logs_plugin__text__regex_tags__v8_sqlite_storage PRIVATE
    gtest_main
    fluxion_logs_plugin__text__regex_tags__v8_interface
    fluxion_logs_plugin__text__regex_tags__v8
)

# Fix: Apply the bin directory to BOTH test executables
set_target_properties(
    test__fluxion_logs_plugin__text__regex_tags__v8 
    unit_tests__fluxion_logs_plugin__text__regex_tags__v8_sqlite_storage 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    # Fix: Changed v1 to v8
    add_custom_command(TARGET test__fluxion_logs_plugin__text__regex_tags__v8 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/bin/plugins/logs/$<TARGET_FILE_NAME:fluxion_logs_plugin__text__regex_tags__v8>"
            $<TARGET_FILE_DIR:test__fluxion_logs_plugin__text__regex_tags__v8>
        COMMENT "Copying V8 plugin DLL from plugins/logs to test binary directory"
    )
endif()

include(GoogleTest)
gtest_discover_tests(test__fluxion_logs_plugin__text__regex_tags__v8
    DISCOVERY_MODE PRE_TEST
)
gtest_discover_tests(unit_tests__fluxion_logs_plugin__text__regex_tags__v8_sqlite_storage
    DISCOVERY_MODE PRE_TEST
)
