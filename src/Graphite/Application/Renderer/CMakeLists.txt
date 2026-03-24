if(APPLE)
    set(CORE_RENDERER_SOURCES
        Renderer.mm
        Backends/Metal/MetalRenderer.mm
        Backends/Metal/MetalRendererState.mm
    )
    set_source_files_properties(${CORE_RENDERER_SOURCES} PROPERTIES LANGUAGE OBJCXX)
else()
    set(CORE_RENDERER_SOURCES
        Renderer.cpp
        Backends/Vulkan/VulkanRenderer.cpp
    )
endif()

add_library(graphite_core_renderer STATIC ${CORE_RENDERER_SOURCES})

set_target_properties(graphite_core_renderer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(graphite_core_renderer PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/external
    ${PROJECT_SOURCE_DIR}/external/imgui
    ${PROJECT_SOURCE_DIR}/external/backends
)

if(APPLE)
    target_compile_definitions(graphite_core_renderer PRIVATE GRAPHITE_USE_METAL_API)
else()
    target_compile_definitions(graphite_core_renderer PRIVATE GRAPHITE_USE_VULKAN_API)
endif()

if(NOT APPLE)
    find_package(Vulkan REQUIRED)
    target_link_libraries(graphite_core_renderer PRIVATE 
        fluxion_compile_options
        imgui
        graphite_core_logger
        Vulkan::Vulkan
    )
else()
    target_link_libraries(graphite_core_renderer PRIVATE 
        fluxion_compile_options
        imgui
        graphite_core_logger
    )
endif()
