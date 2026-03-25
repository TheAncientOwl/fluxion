/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MetalRenderer.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Metal implementation of @see Graphite/Renderer.hpp.
/// Adaptation of @see external/imgui/examples/example_glfw_Metal/main.mm
///

#pragma once

#include <memory>

#include "Graphite/Renderer/Renderer.hpp"

namespace Graphite::Application::Renderer::Backends::Metal {

class MetalRenderer : public IRenderer
{
public:
    MetalRenderer();
    ~MetalRenderer() override;

    void Init(Graphite::Application::WindowConfiguration const& window_configuration) override;
    void Render(std::shared_ptr<IRenderable> user_interface) override;
    void Cleanup() override;

private:
    struct State;
    std::unique_ptr<State> m_state{nullptr};
    bool m_cleaned_up{false};
};

} // namespace Graphite::Application::Renderer::Backends::Metal
