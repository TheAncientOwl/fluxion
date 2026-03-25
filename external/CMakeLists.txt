# --- 1. ImGui ---
set(IMGUI_DIR "${PROJECT_SOURCE_DIR}/external/imgui")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

if(APPLE)
    list(APPEND IMGUI_SOURCES ${IMGUI_DIR}/backends/imgui_impl_metal.mm)
else()
    list(APPEND IMGUI_SOURCES ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp)
endif()

add_library(external_imgui STATIC ${IMGUI_SOURCES})

target_include_directories(external_imgui SYSTEM PUBLIC ${IMGUI_DIR})

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

target_include_directories(external_imgui PUBLIC ${glfw_SOURCE_DIR}/include)

# --- OS-Specific Linking ---
if(APPLE)
    set_source_files_properties(${IMGUI_DIR}/backends/imgui_impl_metal.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
    target_compile_options(external_imgui PRIVATE -Wno-arc-bridge-casts-disallowed-in-nonarc)

    target_link_libraries(external_imgui PUBLIC glfw "-framework Cocoa" "-framework Metal" "-framework MetalKit" "-framework QuartzCore")
else()
    # Handle Vulkan for both Windows and Linux using the standard Finder
    find_package(Vulkan REQUIRED)
    target_link_libraries(external_imgui PUBLIC glfw Vulkan::Vulkan)
endif()

# --- 2. IconsCodicons ---
add_library(external_icons_codicons INTERFACE)

target_include_directories(external_icons_codicons INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/icons
)
