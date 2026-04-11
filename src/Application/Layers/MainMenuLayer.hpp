/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MainMenuLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief Render App's menu.
///

#pragma once

#include "Fluxion.hpp"
#include "Fluxion/Application/Data/AppState.hpp"
#include "Graphite/Application/Layers/TLayer.hpp"
#include "Graphite/Common/UI/FileDialog.hpp"

namespace Fluxion::Application::Layers {

class MainMenuLayer : public Graphite::Application::Layers::TLayer<AppState, EFluxionAction>
{
public:
    static std::string_view GetLayerName() noexcept;
    std::string_view GetName() const noexcept override;

    MainMenuLayer(
        Fluxion::Application::FluxionApplication::Ptr application,
        Graphite::Application::Layers::ZIndex const z_index);

    void OnAdd() override;
    void OnIterate() override;
    void OnRender() override;
    void OnRemove() override;

private:
    void RenderMenu();

    Graphite::Common::UI::FileDialog m_file_dialog;
    std::filesystem::path m_last_file_dialog_path;
};

} // namespace Fluxion::Application::Layers
