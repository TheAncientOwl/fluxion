# --- Plugin Tests ---
add_executable(test__fluxion_logs_plugin__text__regex_tags__v9
    PluginTest.cpp
)

target_link_libraries(test__fluxion_logs_plugin__text__regex_tags__v9 PUBLIC
    external_imgui

    fluxion_api_testing_interface

    fluxion_logs_plugin__text__regex_tags__v9_interface
    fluxion_logs_plugin__text__regex_tags__v9
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

add_executable(unit_tests__fluxion_logs_plugin__text__regex_tags__v9_sqlite_storage
    SQLiteStorageTest.cpp
    ../src/Data.cpp
    ../src/SQLite/SQLiteStorage.cpp
)

target_include_directories(unit_tests__fluxion_logs_plugin__text__regex_tags__v9_sqlite_storage PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(unit_tests__fluxion_logs_plugin__text__regex_tags__v9_sqlite_storage PRIVATE
    gtest_main
    fluxion_logs_plugin__text__regex_tags__v9_interface
    fluxion_logs_plugin__text__regex_tags__v9
)

# Fix 1: Apply the bin directory to BOTH executables
set_target_properties(
    test__fluxion_logs_plugin__text__regex_tags__v9 
    unit_tests__fluxion_logs_plugin__text__regex_tags__v9_sqlite_storage
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    # Fix 2: Changed v1 to v9
    add_custom_command(TARGET test__fluxion_logs_plugin__text__regex_tags__v9 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/bin/plugins/logs/$<TARGET_FILE_NAME:fluxion_logs_plugin__text__regex_tags__v9>"
            $<TARGET_FILE_DIR:test__fluxion_logs_plugin__text__regex_tags__v9>
        COMMENT "Copying V9 plugin DLL from plugins/logs to test binary directory"
    )
endif()

include(GoogleTest)
gtest_discover_tests(test__fluxion_logs_plugin__text__regex_tags__v9
    DISCOVERY_MODE PRE_TEST
)
gtest_discover_tests(unit_tests__fluxion_logs_plugin__text__regex_tags__v9_sqlite_storage
    DISCOVERY_MODE PRE_TEST
)
