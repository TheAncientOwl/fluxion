# --- 1. Interface ---
add_library(graphite_common_interface INTERFACE)

target_include_directories(graphite_common_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(graphite_common_interface INTERFACE
    external_nlohmann_json
)

# --- 2. Implementation ---
add_library(graphite_common_impl STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UniqueID.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ImGuiHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DynamicLibrary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ThemeSerializer.cpp
)

set_target_properties(graphite_common_impl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# target_precompile_headers(graphite_common_impl REUSE_FROM fluxion_pch)

target_link_libraries(graphite_common_impl PRIVATE
    project_compile_options
    external_imgui
    external_icons_codicons
    graphite_common_interface
    graphite_logger_interface
    external_nlohmann_json
)

target_include_directories(graphite_common_impl PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)
