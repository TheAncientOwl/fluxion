# --- Plugin Tests ---
add_executable(test__fluxion_logs_plugin__text__regex_tags__v8
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
    SQLiteStorageTest.cpp
    ../src/Data.cpp
    ../src/SQLite/SQLiteStorage.cpp
)

target_include_directories(unit_tests__fluxion_logs_plugin__text__regex_tags__v8_sqlite_storage PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(unit_tests__fluxion_logs_plugin__text__regex_tags__v8_sqlite_storage PRIVATE
    gtest_main
    fluxion_logs_plugin__text__regex_tags__v8_interface
    fluxion_logs_plugin__text__regex_tags__v8
)

include(GoogleTest)
gtest_discover_tests(
    unit_tests__fluxion_logs_plugin__text__regex_tags__v8_sqlite_storage
    DISCOVERY_MODE PRE_TEST
)

