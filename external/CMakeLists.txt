# ImGui
set(IMGUI_SOURCES
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui_tables.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    # ${PROJECT_SOURCE_DIR}/external/imgui/imgui_demo.cpp
)

if(APPLE)
    list(APPEND IMGUI_SOURCES
        ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
        ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_metal.mm
    )
    set_source_files_properties(
        ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_metal.mm
        PROPERTIES COMPILE_OPTIONS "-fobjc-arc"
    )
else()
    list(APPEND IMGUI_SOURCES
        ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
        ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_vulkan.cpp
    )
endif()

add_library(imgui SHARED ${IMGUI_SOURCES})
target_include_directories(imgui SYSTEM PUBLIC
    ${PROJECT_SOURCE_DIR}/external/imgui
)

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

target_include_directories(imgui PUBLIC ${GLFW_INCLUDE_DIRS})

if(APPLE)
    target_compile_options(imgui PRIVATE
        -Wno-arc-bridge-casts-disallowed-in-nonarc
    )

    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(METAL_LIBRARY Metal REQUIRED)
    find_library(METALKIT_LIBRARY MetalKit REQUIRED)
    find_library(QUARTZCORE_LIBRARY QuartzCore REQUIRED)

    target_link_libraries(imgui PRIVATE
        glfw
        ${COCOA_LIBRARY}
        ${METAL_LIBRARY}
        ${METALKIT_LIBRARY}
        ${QUARTZCORE_LIBRARY}
    )
elseif(WIN32)
    # Vulkan SDK paths
    set(Vulkan_INCLUDE_DIR "D:/VulkanSDK/1.4.341.1/Include")
    set(Vulkan_LIBRARY "D:/VulkanSDK/1.4.341.1/Lib/vulkan-1.lib")

    target_include_directories(imgui PUBLIC
        ${glfw_SOURCE_DIR}/include
        ${Vulkan_INCLUDE_DIR}
    )

    target_link_libraries(imgui
        PRIVATE
        glfw
        ${Vulkan_LIBRARY}
    )
    
    target_include_directories(imgui PUBLIC
        ${Vulkan_INCLUDE_DIR}
    )
    target_link_libraries(imgui PRIVATE ${Vulkan_LIBRARY})
else()
    find_package(Vulkan REQUIRED)
    
    target_link_libraries(imgui PRIVATE 
        glfw 
        Vulkan::Vulkan
    )
endif()
