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

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
    ${PROJECT_SOURCE_DIR}/external/imgui
)
