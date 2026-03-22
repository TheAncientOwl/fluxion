/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MetalRendererState.mm
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief State of @see MetalRenderer.hpp
///

#include <GLFW/glfw3.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include "MetalRenderer.hpp"

namespace Graphite::Application::Renderer::Backends::Metal {

struct MetalRenderer::State
{
    GLFWwindow* window{nullptr};
    CAMetalLayer* layer{nullptr};
    id<MTLDevice> device{nullptr};
    id<MTLCommandQueue> commandQueue{nullptr};
    MTLRenderPassDescriptor* renderPassDescriptor{nullptr};
};

} // namespace Graphite::Application::Renderer::Backends::Metal
