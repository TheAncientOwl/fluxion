/// --------------------------------------------------------------------------
///                     Copyright (c) by Graphite 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/graphite/blob/main/LICENSE
///
/// @file MetalRendererState.mm
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief State of @see MetalRenderer.hpp
///

#include <GLFW/glfw3.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include "MetalRenderer.hpp"

namespace Graphite::Core::Renderer::Backends::Metal {

struct MetalRenderer::State
{
    GLFWwindow* window{nullptr};
    CAMetalLayer* layer{nullptr};
    id<MTLDevice> device{nullptr};
    id<MTLCommandQueue> commandQueue{nullptr};
    MTLRenderPassDescriptor* renderPassDescriptor{nullptr};
};

} // namespace Graphite::Core::Renderer::Backends::Metal
