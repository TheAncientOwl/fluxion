/// --------------------------------------------------------------------------
///                     Copyright (c) by Graphite 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/graphite/blob/main/LICENSE
///
/// @file Renderer.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation of @see Renderer.hpp::CreateRenderer.
///

#ifdef GRAPHITE_USE_VULKAN_API
#include <memory>

#include "Backends/Vulkan/VulkanRenderer.hpp"
#include "Renderer.hpp"

namespace Graphite::Core::Renderer {

std::unique_ptr<IRenderer> CreateRenderer()
{
    return std::make_unique<Backends::Vulkan::VulkanRenderer>();
}
} // namespace Graphite::Core::Renderer

#endif
