/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Renderer.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Create IRenderer based on build platform.
///

#pragma once

#include <memory>

#include "Graphite/Application/WindowConfiguration.hpp"

namespace Graphite::Application::Renderer {

class IRenderable
{
public:
    virtual void Render() = 0;
    virtual ~IRenderable() = default;
};

class IRenderer
{
public:
    virtual void Init(Graphite::Application::WindowConfiguration const& window_configuration) = 0;
    virtual void Render(std::shared_ptr<IRenderable> user_interface) = 0;
    virtual void Cleanup() = 0;
    virtual ~IRenderer() = default;
};

std::unique_ptr<IRenderer> CreateRenderer();

} // namespace Graphite::Application::Renderer
