/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ILayer.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief App layer.
///

#pragma once

#include <memory>
#include <string_view>

#include "Core/Common/UniqueID.hpp"

namespace Graphite::Core::Application {

template <typename ApplicationState>
class TGraphiteApplication;

template <typename ApplicationState>
class ILayer
{
public:
    using Ptr = std::unique_ptr<ILayer<ApplicationState>>;

    ILayer(std::shared_ptr<Graphite::Core::Application::TGraphiteApplication<ApplicationState>> application)
        : m_application{std::move(application)}
        , m_layer_uid{Graphite::Core::Common::UniqueID::generate()} {};

    ILayer(
        std::shared_ptr<Graphite::Core::Application::TGraphiteApplication<ApplicationState>> application,
        Graphite::Core::Common::UniqueID uid)
        : m_application{std::move(application)}, m_layer_uid{std::move(uid)} {};

    virtual ~ILayer() = default;

    inline Graphite::Core::Common::UniqueID const& GetUID() const noexcept { return m_layer_uid; }

private:
    friend class Graphite::Core::Application::TGraphiteApplication<ApplicationState>;

    virtual std::string_view GetName() const noexcept = 0;

    virtual void OnPush() = 0;
    virtual void OnRender() = 0;
    virtual void OnPop() = 0;

protected:
    std::shared_ptr<TGraphiteApplication<ApplicationState>> m_application{nullptr};
    Graphite::Core::Common::UniqueID m_layer_uid{};
};

} // namespace Graphite::Core::Application
