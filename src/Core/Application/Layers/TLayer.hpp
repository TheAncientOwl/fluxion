/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TLayer.hpp
/// @author Alexandru Delegeanu
/// @version 1.5
/// @brief App layer.
///

#pragma once

#include <memory>
#include <string_view>

#include "Core/Common/UniqueID.hpp"

namespace Graphite::Core::Application {

template <typename ApplicationState>
class TGraphiteApplication;

} // namespace Graphite::Core::Application

namespace Graphite::Core::Application::Layers {

using ZIndex = std::uint8_t;

template <typename ApplicationState>
class TLayer
{
public:
    using Ptr = std::unique_ptr<TLayer<ApplicationState>>;
    using ApplicationPtr =
        std::shared_ptr<Graphite::Core::Application::TGraphiteApplication<ApplicationState>>;

    TLayer(
        std::shared_ptr<Graphite::Core::Application::TGraphiteApplication<ApplicationState>> application,
        ZIndex const z_index)
        : m_z_index{z_index}
        , m_layer_id{Graphite::Core::Common::UniqueID::Generate()}
        , m_application{std::move(application)}
    {
    }

    TLayer(
        std::shared_ptr<Graphite::Core::Application::TGraphiteApplication<ApplicationState>> application,
        ZIndex const zindex,
        Graphite::Core::Common::UniqueID id)
        : m_z_index{zindex}, m_layer_id{std::move(id)}, m_application{std::move(application)} {};

    virtual ~TLayer() = default;

    virtual inline bool IsActive() const noexcept { return true; };
    virtual inline void SetIsActive(bool const /* active */) {};

    inline Graphite::Core::Common::UniqueID const& GetID() const noexcept { return m_layer_id; }
    inline bool GetZIndex() const noexcept { return m_z_index; }

protected:
    friend class Graphite::Core::Application::TGraphiteApplication<ApplicationState>;

    virtual std::string_view GetName() const noexcept = 0;

    virtual void OnAdd() = 0;
    virtual void OnRender() = 0;
    virtual void OnRemove() = 0;

protected:
    ZIndex m_z_index{0};
    Graphite::Core::Common::UniqueID m_layer_id{};
    ApplicationPtr m_application{nullptr};
};

} // namespace Graphite::Core::Application::Layers
