/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MainMenuView.hpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief Render App's menu.
///

#pragma once

#include "Fluxion.hpp"
#include "Fluxion/Data/AppState.hpp"
#include "Graphite/Application/Views/TView.hpp"
#include "Graphite/Common/UI/FileDialog.hpp"

namespace Fluxion::Application::Views {

class MainMenuView : public Graphite::Application::Views::TView<AppState, EFluxionAction>
{
public:
    static std::string_view GetViewName() noexcept;
    std::string_view GetName() const noexcept override;

    MainMenuView(
        Fluxion::Application::FluxionApplication::Ptr application,
        Graphite::Application::Views::RenderPriority const render_priority);

    void OnAdd() override;
    void OnIterate() override;
    void OnRender() override;
    void OnRemove() override;

private:
    void RenderMenu();

    Graphite::Common::UI::FileDialog m_file_dialog;
    std::filesystem::path m_last_file_dialog_path;
};

} // namespace Fluxion::Application::Views
