/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ILayer.hpp
/// @author Alexandru Delegeanu
/// @version 1.1
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
class ILayer
{
public:
    using Ptr = std::unique_ptr<ILayer<ApplicationState>>;

    ILayer(
        std::shared_ptr<Graphite::Core::Application::TGraphiteApplication<ApplicationState>> application,
        Layer::ZIndex const z_index)
        : m_application{std::move(application)}
        , m_is_active{true}
        , m_z_index{z_index}
        , m_layer_uid{Graphite::Core::Common::UniqueID::Generate()} {};

    ILayer(
        std::shared_ptr<Graphite::Core::Application::TGraphiteApplication<ApplicationState>> application,
        Graphite::Core::Common::UniqueID uid)
        : m_application{std::move(application)}, m_layer_uid{std::move(uid)} {};

    virtual ~ILayer() = default;

    inline Graphite::Core::Common::UniqueID const& GetUID() const noexcept { return m_layer_uid; }

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
    std::shared_ptr<TGraphiteApplication<ApplicationState>> m_application{nullptr};
    Graphite::Core::Common::UniqueID m_layer_uid{};
    bool m_is_active{true};
    Layer::ZIndex m_z_index{0};
};

} // namespace Graphite::Core::Application
