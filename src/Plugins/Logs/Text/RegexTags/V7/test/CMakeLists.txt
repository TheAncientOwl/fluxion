# --- Plugin Tests ---
add_executable(test__fluxion_logs_plugin__text__regex_tags__v7
    PluginTest.cpp
)

target_link_libraries(test__fluxion_logs_plugin__text__regex_tags__v7 PUBLIC
    external_imgui

    fluxion_api_testing_interface
    fluxion_api_testing_impl

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
)

target_include_directories(unit_tests__fluxion_logs_plugin__text__regex_tags__v7 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(unit_tests__fluxion_logs_plugin__text__regex_tags__v7 PRIVATE
    gtest_main
    fluxion_logs_plugin__text__regex_tags__v7
)

include(GoogleTest)
gtest_discover_tests(unit_tests__fluxion_logs_plugin__text__regex_tags__v7)
