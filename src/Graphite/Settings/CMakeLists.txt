add_library(graphite_settings_interface INTERFACE)

target_include_directories(graphite_settings_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(graphite_settings_interface INTERFACE 
    external_nlohmann_json
)

# --- 2. Implementation ---
add_library(graphite_settings_impl STATIC
    src/SettingsManager.cpp
)

target_include_directories(graphite_settings_impl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(graphite_settings_impl PUBLIC
    graphite_settings_interface
    external_nlohmann_json
)

target_compile_features(graphite_settings_impl PRIVATE cxx_std_23)
