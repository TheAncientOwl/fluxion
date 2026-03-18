/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MetalRenderer.mm
/// @author Alexandru Delegeanu
/// @version 1.1
/// @brief Implementation of @see MetalRenderer.hpp.
///

#include <exception>

#include "Core/Application/WindowConfiguration.hpp"
#include "Core/Logger/Logger.hpp"

#include "MetalRenderer.hpp"
#include "MetalRendererState.mm"

#include <stdio.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_metal.h"
#include "imgui.h"

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

static void glfw_error_callback(int error, const char* description)
{
    LOG_ERROR("Glfw Error {}: {}", error, description);
}

namespace Graphite::Core::Renderer::Backends::Metal {

MetalRenderer::MetalRenderer() : m_state{std::make_unique<State>()}, m_cleaned_up{false}
{
    LOG_SCOPE("");
}

MetalRenderer::~MetalRenderer()
{
    LOG_SCOPE("");
    Cleanup();
}

void MetalRenderer::Init(Graphite::Core::Application::WindowConfiguration const& window_configuration)
{
    LOG_SCOPE("");

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        std::terminate();

    // Create window with graphics context
    float main_scale =
        ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_state->window = glfwCreateWindow(
        static_cast<int>(window_configuration.width * main_scale),
        static_cast<int>(window_configuration.height * main_scale),
        window_configuration.title.data(),
        nullptr,
        nullptr);
    if (m_state->window == nullptr)
        std::terminate();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(
        main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style
                     // scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi =
        main_scale; // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true
                    // automatically overrides this for every window depending on the current monitor)

    m_state->device = MTLCreateSystemDefaultDevice();
    m_state->commandQueue = [m_state->device newCommandQueue];

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_state->window, true);
    ImGui_ImplMetal_Init(m_state->device);

    NSWindow* nswin = glfwGetCocoaWindow(m_state->window);
    m_state->layer = [CAMetalLayer layer];
    m_state->layer.device = m_state->device;
    m_state->layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = m_state->layer;
    nswin.contentView.wantsLayer = YES;

    m_state->renderPassDescriptor = [MTLRenderPassDescriptor new];
}

void MetalRenderer::Render(std::shared_ptr<IRenderable> user_interface)
{
    LOG_SCOPE("Main Render Loop");

    while (!glfwWindowShouldClose(m_state->window))
    {
        @autoreleasepool
        {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application,
            // or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
            // application, or clear/overwrite your copy of the keyboard data. Generally you may always
            // pass all inputs to dear imgui, and hide them from your application based on those two flags.
            glfwPollEvents();

            int width, height;
            glfwGetFramebufferSize(m_state->window, &width, &height);
            m_state->layer.drawableSize = CGSizeMake(width, height);
            id<CAMetalDrawable> drawable = [m_state->layer nextDrawable];

            id<MTLCommandBuffer> commandBuffer = [m_state->commandQueue commandBuffer];
            m_state->renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
            m_state->renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            m_state->renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            m_state->renderPassDescriptor.colorAttachments[0].clearColor =
                MTLClearColorMake(0.45, 0.55, 0.60, 1.0);
            id<MTLRenderCommandEncoder> renderEncoder =
                [commandBuffer renderCommandEncoderWithDescriptor:m_state->renderPassDescriptor];
            [renderEncoder pushDebugGroup:@"ImGui demo"];

            // Start the Dear ImGui frame
            ImGui_ImplMetal_NewFrame(m_state->renderPassDescriptor);
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            user_interface->Render();

            // Rendering
            ImGui::Render();
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);

            [renderEncoder popDebugGroup];
            [renderEncoder endEncoding];

            [commandBuffer presentDrawable:drawable];
            [commandBuffer commit];
        }
    }
};

void MetalRenderer::Cleanup()
{
    LOG_SCOPE("");

    if (!m_cleaned_up)
    {
        m_cleaned_up = true;
        ImGui_ImplMetal_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(m_state->window);
        glfwTerminate();
    }
};

} // namespace Graphite::Core::Renderer::Backends::Metal
