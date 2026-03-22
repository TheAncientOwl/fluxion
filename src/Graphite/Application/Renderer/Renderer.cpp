/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Renderer.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Implementation of @see Renderer.hpp::CreateRenderer.
///

#ifdef GRAPHITE_USE_VULKAN_API
#include <memory>

#include "Backends/Vulkan/VulkanRenderer.hpp"
#include "Renderer.hpp"

namespace Graphite::Application::Renderer {

std::unique_ptr<IRenderer> CreateRenderer()
{
    return std::make_unique<Backends::Vulkan::VulkanRenderer>();
}
} // namespace Graphite::Application::Renderer

#endif
