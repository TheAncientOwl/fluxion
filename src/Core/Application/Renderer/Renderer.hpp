/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Renderer.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Create IRenderer based on buid platform.
///

#pragma once

#include <memory>

#include "Core/Application/WindowConfiguration.hpp"

namespace Graphite::Core::Renderer {

class IRenderable
{
public:
    virtual void Render() = 0;
    virtual ~IRenderable() = default;
};

class IRenderer
{
public:
    virtual void Init(Graphite::Core::Application::WindowConfiguration const& window_configuration) = 0;
    virtual void Render(std::shared_ptr<IRenderable> user_interface) = 0;
    virtual void Cleanup() = 0;
    virtual ~IRenderer() = default;
};

std::unique_ptr<IRenderer> CreateRenderer();

} // namespace Graphite::Core::Renderer
