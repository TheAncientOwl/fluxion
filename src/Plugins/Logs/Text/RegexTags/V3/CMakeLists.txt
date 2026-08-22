# --- 0. Fetch Dependencies ---
include(FetchContent)

set(ABSL_PROPAGATE_CXX_STANDARD ON)
set(ABSL_ENABLE_INSTALL OFF)
FetchContent_Declare(
    absl
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG        20260817.0
)
FetchContent_MakeAvailable(absl)

set(RE2_BUILD_TESTING OFF)
set(RE2_INSTALL OFF)
FetchContent_Declare(
    re2
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(re2)

# --- 1. Interface ---
add_library(fluxion_logs_plugin__text__regex_tags__v3_interface INTERFACE)

target_include_directories(fluxion_logs_plugin__text__regex_tags__v3_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(fluxion_logs_plugin__text__regex_tags__v3_interface INTERFACE
    re2::re2
)

# --- 2. Implementation ---
add_library(fluxion_logs_plugin__text__regex_tags__v3 SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CSV/Wrapper/Wrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ApplyFilters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DisableFilters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GetLogs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GetNextLog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GetPrevLog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GetTableHeader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GetTotalLogs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ImportLogs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OnEnable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OnDisable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RegexTags.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderMenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Progress.cpp
)

set_target_properties(fluxion_logs_plugin__text__regex_tags__v3 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
)

target_link_libraries(fluxion_logs_plugin__text__regex_tags__v3 PUBLIC
    project_compile_options
    external_icons_codicons
    external_miocsv
    graphite_interface
    graphite_impl
    fluxion_api_interface
    fluxion_api_impl
    fluxion_logs_plugin__text__regex_tags__v3_interface
    re2::re2
)

target_include_directories(fluxion_logs_plugin__text__regex_tags__v3 PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/Graphite/Logger/include
    ${PROJECT_SOURCE_DIR}/external/imgui
)

# --- 3. Install to ~/.fluxion/plugins/logs ---
add_custom_command(TARGET fluxion_logs_plugin__text__regex_tags__v3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $ENV{HOME}/.fluxion/plugins/logs
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:fluxion_logs_plugin__text__regex_tags__v3> $ENV{HOME}/.fluxion/plugins/logs
    COMMENT "Copying logs plugin to ~/.fluxion/plugins/logs"
)

# Also install on every cmake configure to ensure plugin is available even if not rebuilt
add_custom_target(install_regex_text_v3_plugin ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory $ENV{HOME}/.fluxion/plugins/logs
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:fluxion_logs_plugin__text__regex_tags__v3> $ENV{HOME}/.fluxion/plugins/logs
    DEPENDS fluxion_logs_plugin__text__regex_tags__v3
    COMMENT "Installing logs plugin to ~/.fluxion/plugins/logs"
)

add_subdirectory(test)
