add_subdirectory(Logs)

# --- 1. Interface ---
add_library(fluxion_sentinel_plugins_interface INTERFACE)
target_link_libraries(fluxion_sentinel_plugins_interface INTERFACE
    fluxion_sentinel_plugin__logs_interface
)

# --- 2. Implementation
add_library(fluxion_sentinel_plugins INTERFACE)
target_link_libraries(fluxion_sentinel_plugins INTERFACE
    fluxion_sentinel_plugin__logs
)
