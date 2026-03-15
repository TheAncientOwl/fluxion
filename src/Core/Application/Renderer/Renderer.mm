/// --------------------------------------------------------------------------
///                     Copyright (c) by Graphite 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/graphite/blob/main/LICENSE
///
/// @file Renderer.mm
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation of @see Renderer.hpp::CreateRenderer.
///

#ifdef GRAPHITE_USE_METAL_API
#include "Renderer.hpp"

#include "Core/Application/Renderer/Backends/Metal/MetalRenderer.hpp"

namespace Graphite::Core::Renderer {

std::unique_ptr<IRenderer> CreateRenderer()
{
    return std::make_unique<Backends::Metal::MetalRenderer>();
}

} // namespace Graphite::Core::Renderer

#endif
