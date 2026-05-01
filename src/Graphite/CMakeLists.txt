add_subdirectory(Logger)
add_subdirectory(Common)
add_subdirectory(Application)
add_subdirectory(Settings)

add_library(graphite_interface INTERFACE)
target_link_libraries(graphite_interface INTERFACE
    graphite_logger_interface
    graphite_common_interface
    graphite_renderer_interface
    graphite_application_interface
    graphite_settings_interface
)

add_library(graphite_impl INTERFACE)
target_link_libraries(graphite_impl INTERFACE
    graphite_logger_impl
    graphite_common_impl
    graphite_renderer_impl
    graphite_settings_impl
)
