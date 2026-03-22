/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TSoftMenuCloseableLayer.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Layer that is removed / added on close / open.
///

#pragma once

#include "TLayer.hpp"

namespace Graphite::Application::Layers {

template <typename ApplicationState>
class TSoftMenuCloseableLayer : public TLayer<ApplicationState>
{
public:
    virtual inline std::string_view GetDisplayName() const noexcept = 0;

    virtual bool IsActive() const noexcept = 0;
    virtual void SetIsActive(bool active) = 0;

    TSoftMenuCloseableLayer(
        std::shared_ptr<Graphite::Application::TGraphiteApplication<ApplicationState>> application,
        ZIndex const z_index)
        : TLayer<ApplicationState>(std::move(application), z_index)
    {
    }

    TSoftMenuCloseableLayer(
        std::shared_ptr<Graphite::Application::TGraphiteApplication<ApplicationState>> application,
        ZIndex const zindex,
        Graphite::Common::UniqueID id)
        : TLayer<ApplicationState>(std::move(application), zindex, std::move(id))
    {
    }
};

} // namespace Graphite::Application::Layers
