# --- 1. Interface ---
add_library(graphite_renderer_interface INTERFACE)

target_include_directories(graphite_renderer_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(graphite_renderer_interface INTERFACE
    graphite_application_interface
)

# --- 2. Implementation ---
if(APPLE)
    set(CORE_RENDERER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Backends/Metal/MetalRenderer.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Backends/Metal/MetalRendererState.mm
    )
    set_source_files_properties(${CORE_RENDERER_SOURCES} PROPERTIES LANGUAGE OBJCXX)
else()
    set(CORE_RENDERER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Backends/Vulkan/VulkanRenderer.cpp
    )
endif()

add_library(graphite_renderer_impl STATIC ${CORE_RENDERER_SOURCES})

set_target_properties(graphite_renderer_impl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(graphite_renderer_impl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(APPLE)
    target_compile_definitions(graphite_renderer_impl PRIVATE GRAPHITE_USE_METAL_API)
else()
    target_compile_definitions(graphite_renderer_impl PRIVATE GRAPHITE_USE_VULKAN_API)
endif()

if(NOT APPLE)
    find_package(Vulkan REQUIRED)
    target_link_libraries(graphite_renderer_impl PRIVATE 
        external_imgui
        Vulkan::Vulkan
        project_compile_options
        graphite_renderer_interface
        graphite_logger_interface
    )
else()
    target_link_libraries(graphite_renderer_impl PRIVATE 
        external_imgui
        project_compile_options
        graphite_logger_interface
        graphite_renderer_interface
    )
endif()
