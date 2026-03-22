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

add_library(graphite_core_renderer SHARED ${CORE_RENDERER_SOURCES})

set_target_properties(graphite_core_renderer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_precompile_headers(graphite_core_renderer REUSE_FROM fluxion_pch)

target_include_directories(graphite_core_renderer PUBLIC
    ${GLFW_INCLUDE_DIRS}
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

    target_compile_options(graphite_core_renderer PRIVATE
        -Wno-arc-bridge-casts-disallowed-in-nonarc
    )
else()
    message(STATUS "Graphite::Application::Renderer::GRAPHITE_USE_VULKAN_API")
    target_compile_definitions(graphite_core_renderer PRIVATE GRAPHITE_USE_VULKAN_API)
endif()

# 3. Link dependencies
include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.10
)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

target_link_libraries(imgui glfw)

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(METAL_LIBRARY Metal REQUIRED)
    find_library(METALKIT_LIBRARY MetalKit REQUIRED)
    find_library(QUARTZCORE_LIBRARY QuartzCore REQUIRED)

    target_link_libraries(graphite_core_renderer PRIVATE
        glfw
        ${COCOA_LIBRARY}
        ${METAL_LIBRARY}
        ${METALKIT_LIBRARY}
        ${QUARTZCORE_LIBRARY}
    )
else()
    if(WIN32 AND NOT APPLE)
        set(Vulkan_INCLUDE_DIR "D:/VulkanSDK/1.4.341.1/Include")
        set(Vulkan_LIBRARY "D:/VulkanSDK/1.4.341.1/Lib/vulkan-1.lib")
    endif()
    find_package(Vulkan REQUIRED)
    target_include_directories(graphite_core_renderer PUBLIC ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(graphite_core_renderer PRIVATE glfw Vulkan::Vulkan)
endif()
