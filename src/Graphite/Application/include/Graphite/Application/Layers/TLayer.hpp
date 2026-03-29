/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TLayer.hpp
/// @author Alexandru Delegeanu
/// @version 1.7
/// @brief App layer.
///

#pragma once

#include <memory>
#include <string_view>

#include "Graphite/Common/UniqueID.hpp"

namespace Graphite::Application {

template <typename ApplicationState, typename ActionEnum>
class TGraphiteApplication;

} // namespace Graphite::Application

namespace Graphite::Application::Layers {

using ZIndex = std::uint8_t;

template <typename ApplicationState, typename ActionEnum>
class TLayer
{
public:
    using Ptr = std::unique_ptr<TLayer<ApplicationState, ActionEnum>>;
    using ApplicationPtr =
        std::shared_ptr<Graphite::Application::TGraphiteApplication<ApplicationState, ActionEnum>>;

    TLayer(
        std::shared_ptr<Graphite::Application::TGraphiteApplication<ApplicationState, ActionEnum>> application,
        ZIndex const z_index)
        : m_z_index{z_index}
        , m_layer_id{Graphite::Common::UniqueID::Generate()}
        , m_application{std::move(application)}
    {
    }

    TLayer(
        std::shared_ptr<Graphite::Application::TGraphiteApplication<ApplicationState, ActionEnum>> application,
        ZIndex const zindex,
        Graphite::Common::UniqueID id)
        : m_z_index{zindex}, m_layer_id{std::move(id)}, m_application{std::move(application)} {};

    virtual ~TLayer() = default;

    virtual inline bool IsActive() const noexcept { return true; };
    virtual inline void SetIsActive(bool const /* active */) {};

    inline Graphite::Common::UniqueID const& GetID() const noexcept { return m_layer_id; }
    inline bool GetZIndex() const noexcept { return m_z_index; }

    ApplicationPtr GetApplication() { return m_application; }

protected:
    friend class Graphite::Application::TGraphiteApplication<ApplicationState, ActionEnum>;

    virtual std::string_view GetName() const noexcept = 0;

    /**
     * @brief Called when the layer is added to the application
     * @note Complementary to @see OnRemove
     */
    virtual void OnAdd() = 0;

    /**
     * @brief Layer business logic lives here.
     * @note Called every frame before @see OnRender.
     */
    virtual void OnIterate() = 0;

    /**
     * @brief Layer UI logic lives here
     * @note Called after @see OnIterate
     */
    virtual void OnRender() = 0;

    /**
     * @brief Called when the layer is removed from the application
     * @note Complementary to @see OnAdd
     */
    virtual void OnRemove() = 0;

protected:
    ZIndex m_z_index{0};
    Graphite::Common::UniqueID m_layer_id{};
    ApplicationPtr m_application{nullptr};
};

} // namespace Graphite::Application::Layers
