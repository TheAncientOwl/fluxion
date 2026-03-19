/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BaseLayer.hpp
/// @author Alexandru Delegeanu
/// @version 1.4
/// @brief App layer.
///

#pragma once

#include <memory>
#include <string_view>

#include "Core/Common/UniqueID.hpp"

namespace Graphite::Core::Application {

template <typename ApplicationState>
class TGraphiteApplication;

namespace Layer {

using ZIndex = std::uint8_t;

} // namespace Layer

template <typename ApplicationState>
class BaseLayer
{
public:
    using Ptr = std::unique_ptr<BaseLayer<ApplicationState>>;

    BaseLayer(
        std::shared_ptr<Graphite::Core::Application::TGraphiteApplication<ApplicationState>> application,
        Layer::ZIndex const z_index)
        : m_is_active{true}
        , m_z_index{z_index}
        , m_layer_id{Graphite::Core::Common::UniqueID::Generate()}
        , m_application{std::move(application)}
    {
    }

    BaseLayer(
        std::shared_ptr<Graphite::Core::Application::TGraphiteApplication<ApplicationState>> application,
        Layer::ZIndex const zindex,
        Graphite::Core::Common::UniqueID id)
        : m_application{std::move(application)}
        , m_is_active{true}
        , m_z_index{zindex}
        , m_layer_id{std::move(id)} {};

    virtual ~BaseLayer() = default;

    inline Graphite::Core::Common::UniqueID const& GetID() const noexcept { return m_layer_id; }

    inline bool GetZIndex() const noexcept { return m_z_index; }
    inline bool IsActive() const noexcept { return m_is_active; }
    inline bool Activate() noexcept { m_is_active = true; }
    inline bool Deactivate() noexcept { m_is_active = false; }

private:
    friend class Graphite::Core::Application::TGraphiteApplication<ApplicationState>;

    virtual std::string_view GetName() const noexcept = 0;

    virtual void OnAdd() = 0;
    virtual void OnRender() = 0;
    virtual void OnRemove() = 0;

protected:
    bool m_is_active{true};
    Layer::ZIndex m_z_index{0};
    Graphite::Core::Common::UniqueID m_layer_id{};
    std::shared_ptr<Graphite::Core::Application::TGraphiteApplication<ApplicationState>> m_application{
        nullptr};
};

} // namespace Graphite::Core::Application
