# 1. Create Library
if(APPLE)
    set(CORE_RENDERER_SOURCES
        Renderer.mm
        Backends/Metal/MetalRenderer.mm
        Backends/Metal/MetalRendererState.mm
    )

    # Set Objective-C++ compiler for Metal backend
    set_source_files_properties(
        Renderer.mm
        Backends/Metal/MetalRenderer.mm
        Backends/Metal/MetalRendererState.mm
        PROPERTIES LANGUAGE OBJCXX
    )
else()
    set(CORE_RENDERER_SOURCES
        Renderer.cpp
        Backends/Vulkan/VulkanRenderer.cpp
    )
endif()

add_library(graphite_core_renderer STATIC ${CORE_RENDERER_SOURCES})

set_target_properties(graphite_core_renderer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# target_precompile_headers(graphite_core_renderer REUSE_FROM fluxion_pch)

target_include_directories(graphite_core_renderer PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/external
    ${PROJECT_SOURCE_DIR}/external/imgui
    ${PROJECT_SOURCE_DIR}/external/backends
)

target_link_libraries(graphite_core_renderer PRIVATE
    fluxion_compile_options
    imgui
    graphite_core_logger
)

# 2. Set compile definitions
if(APPLE)
    message(STATUS "Graphite::Application::Renderer::GRAPHITE_USE_METAL_API")
    target_compile_definitions(graphite_core_renderer PRIVATE GRAPHITE_USE_METAL_API)
else()
    message(STATUS "Graphite::Application::Renderer::GRAPHITE_USE_VULKAN_API")
    target_compile_definitions(graphite_core_renderer PRIVATE GRAPHITE_USE_VULKAN_API)
endif()

if (WIN32)
    find_package(Vulkan REQUIRED)
    if(Vulkan_FOUND)
        target_include_directories(graphite_core_renderer PRIVATE ${Vulkan_INCLUDE_DIRS})
        target_link_libraries(graphite_core_renderer PRIVATE ${Vulkan_LIBRARIES})
        target_include_directories(graphite_core_renderer PRIVATE /mingw64/include)
        target_link_libraries(graphite_core_renderer PRIVATE vulkan-1)
    endif()
endif()
