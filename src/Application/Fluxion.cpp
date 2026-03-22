/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion.cpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief Implementation of @see Fluxion.hpp.
///

#include "Fluxion.hpp"

#include "icons/IconsCodicons.h"

#include "Layers/BaseLayer.hpp"
#include "Layers/DebugLayer.hpp"
#include "Layers/FiltersLayer.hpp"
#include "Layers/LogsViewLayer.hpp"
#include "Layers/MainMenuLayer.hpp"

#include "DummyPlugin.hpp"

namespace Fluxion::Application {

FluxionApplication::FluxionApplication(
    Graphite::Application::WindowConfiguration window_configuration,
    AppState initial_state)
    : TGraphiteApplication{std::move(window_configuration), std::move(initial_state)}
{
    LOG_SCOPE("");
}

FluxionApplication::~FluxionApplication()
{
    LOG_SCOPE("");
}

void FluxionApplication::AppInit()
{
    LOG_SCOPE("");
    if (ImGui::GetCurrentContext() == nullptr)
    {
        ImGui::CreateContext();
    }
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    SetupFonts();

    // TODO: this is only for testing purpose during impl
    m_app_state.logs_logic = std::make_unique<DummyPlugin>();

    AddLayer<Layers::BaseLayer>(shared_from_this(), 0);
    AddLayer<Layers::DebugLayer>(
        shared_from_this(), std::numeric_limits<Graphite::Application::Layers::ZIndex>::max());
    AddLayer<Layers::MainMenuLayer>(shared_from_this(), 1);
    AddLayer<Layers::LogsViewLayer>(shared_from_this(), 10);
    AddLayer<Layers::FiltersLayer>(shared_from_this(), 20);
}

void FluxionApplication::SetupFonts()
{
    auto& io = ImGui::GetIO();

    io.Fonts->AddFontFromFileTTF("assets/fonts/JetBrainsMono-Medium.ttf", 15.5f);

    ImFontConfig config;
    config.MergeMode = true;
    config.PixelSnapH = true;
    config.GlyphOffset.y = 2.5f;
    ImWchar const codicon_ranges[] = {ICON_MIN_CI, ICON_MAX_16_CI, 0};
    io.Fonts->AddFontFromFileTTF("assets/fonts/codicon.ttf", 15.5f, &config, codicon_ranges);
}

} // namespace Fluxion::Application
