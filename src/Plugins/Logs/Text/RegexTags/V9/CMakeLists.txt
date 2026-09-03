# --- 0. Fetch Dependencies ---
include(FetchContent)

# --- 0.1. Google RE2 ---
set(ABSL_PROPAGATE_CXX_STANDARD ON)
set(ABSL_ENABLE_INSTALL OFF)
FetchContent_Declare(
    absl
    SYSTEM
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG        20260817.0
)
FetchContent_MakeAvailable(absl)

set(RE2_BUILD_TESTING OFF)
set(RE2_INSTALL OFF)
FetchContent_Declare(
    re2
    SYSTEM
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(re2)

# --- 0.2. SQLite ---
FetchContent_Declare(
    sqlite3
    SYSTEM
    GIT_REPOSITORY https://github.com/sjinks/sqlite3-cmake.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(sqlite3)

# --- 1. Interface ---
add_library(fluxion_logs_plugin__text__regex_tags__v9_interface INTERFACE)

target_include_directories(fluxion_logs_plugin__text__regex_tags__v9_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(fluxion_logs_plugin__text__regex_tags__v9_interface INTERFACE
    re2::re2
    sqlite3
)

# --- 2. Implementation ---
add_library(fluxion_logs_plugin__text__regex_tags__v9 SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SQLite/SQLiteStorage.cpp

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

target_compile_definitions(fluxion_logs_plugin__text__regex_tags__v9 PRIVATE
    GRAPHITE_EXPORTS
)

set_target_properties(fluxion_logs_plugin__text__regex_tags__v9 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/plugins/logs"
)

target_link_libraries(fluxion_logs_plugin__text__regex_tags__v9 PUBLIC
    project_compile_options
    external_icons_codicons
    graphite_interface
    graphite_impl
    fluxion_api_interface
    fluxion_api_impl
    fluxion_logs_plugin__text__regex_tags__v9_interface
    re2::re2
    sqlite3
)

target_compile_options(fluxion_logs_plugin__text__regex_tags__v9 PRIVATE -Wno-nullability-extension)

target_include_directories(fluxion_logs_plugin__text__regex_tags__v9 PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/Graphite/Logger/include
    ${PROJECT_SOURCE_DIR}/external/imgui
)

# --- 3. Install to ~/.fluxion/plugins/logs ---
add_custom_command(TARGET fluxion_logs_plugin__text__regex_tags__v9 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FLUXION_PLUGIN_INSTALL_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:fluxion_logs_plugin__text__regex_tags__v9> "${FLUXION_PLUGIN_INSTALL_DIR}"
    COMMENT "Copying logs plugin to ${FLUXION_PLUGIN_INSTALL_DIR}"
)

# Also install on every cmake configure to ensure plugin is available even if not rebuilt
add_custom_target(install_regextags_v9_plugin ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FLUXION_PLUGIN_INSTALL_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:fluxion_logs_plugin__text__regex_tags__v9> "${FLUXION_PLUGIN_INSTALL_DIR}"
    DEPENDS fluxion_logs_plugin__text__regex_tags__v9
    COMMENT "Installing logs plugin to ${FLUXION_PLUGIN_INSTALL_DIR}"
)

add_subdirectory(test)
