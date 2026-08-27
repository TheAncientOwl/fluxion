/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion.hpp
/// @author Alexandru Delegeanu
/// @version 0.8
/// @brief Fluxion business logic entry point.
///

#pragma once

#include "Fluxion/Data/AppState.hpp"
#include "Graphite/Application/TGraphiteApplication.hpp"

namespace Fluxion::Application {

class FluxionApplication : public Graphite::Application::TGraphiteApplication<AppState, EFluxionAction>
{
public:
    using Ptr = Graphite::Application::TGraphiteApplication<AppState, EFluxionAction>::Ptr;

    ~FluxionApplication();

public:
    /**
     * @brief Used to reset every data log related.
     * @note Generally whenever logs plugin requests for import/filter are invoked.
     */
    void ResetImportedLogsData();

    std::filesystem::path GetHomePath() const;

private:
    friend class Graphite::Application::TGraphiteApplication<AppState, EFluxionAction>;
    FluxionApplication(Graphite::Application::WindowConfiguration window_configuration, AppState initial_state);

    void SetupFonts();

public: // Persistence
    void LoadFiltersFromDisk();
    void SaveFiltersToDisk() const;

    void LoadAppOptionsFromDisk();

    void LoadFiltersSwatchesFromDisk();
    void SaveFiltersSwatchesToDisk() const;

    void SavePluginPathToDisk() const;
    void LoadPluginPathFromDisk();

private:
    void OnInit() override;
    void OnShutdown() override;
    void OnProcessAction(Graphite::Common::Utility::TAppAction<EFluxionAction> const& action) override;
};

} // namespace Fluxion::Application
