/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TView.hpp
/// @author Alexandru Delegeanu
/// @version 1.7
/// @brief App view.
///

#pragma once

#include <memory>
#include <string_view>

#include "Graphite/Common/Utility/UniqueID.hpp"

namespace Graphite::Application {

template <typename ApplicationState, typename ActionEnum>
class TGraphiteApplication;

} // namespace Graphite::Application

namespace Graphite::Application::Views {

using RenderPriority = std::uint8_t;

template <typename ApplicationState, typename ActionEnum>
class TView
{
public:
    using Ptr = std::unique_ptr<TView<ApplicationState, ActionEnum>>;
    using ApplicationPtr =
        std::shared_ptr<Graphite::Application::TGraphiteApplication<ApplicationState, ActionEnum>>;

    TView(
        std::shared_ptr<Graphite::Application::TGraphiteApplication<ApplicationState, ActionEnum>> application,
        RenderPriority const render_priority)
        : m_render_priority{render_priority}
        , m_view_id{Graphite::Common::Utility::UniqueID::Generate()}
        , m_application{std::move(application)}
    {
    }

    TView(
        std::shared_ptr<Graphite::Application::TGraphiteApplication<ApplicationState, ActionEnum>> application,
        RenderPriority const render_priority,
        Graphite::Common::Utility::UniqueID id)
        : m_render_priority{render_priority}
        , m_view_id{std::move(id)}
        , m_application{std::move(application)} {};

    virtual ~TView() = default;

    virtual inline bool IsActive() const noexcept { return true; };
    virtual inline void SetIsActive(bool const /* active */) {};

    inline Graphite::Common::Utility::UniqueID const& GetID() const noexcept { return m_view_id; }
    inline bool GetRenderPriority() const noexcept { return m_render_priority; }

    ApplicationPtr GetApplication() { return m_application; }

protected:
    friend class Graphite::Application::TGraphiteApplication<ApplicationState, ActionEnum>;

    virtual std::string_view GetName() const noexcept = 0;

    /**
     * @brief Called when the view is added to the application
     * @note Complementary to @see OnRemove
     */
    virtual void OnAdd() = 0;

    /**
     * @brief View business logic lives here.
     * @note Called every frame before @see OnRender.
     */
    virtual void OnIterate() = 0;

    /**
     * @brief View UI logic lives here
     * @note Called after @see OnIterate
     */
    virtual void OnRender() = 0;

    /**
     * @brief Called when the view is removed from the application
     * @note Complementary to @see OnAdd
     */
    virtual void OnRemove() = 0;

protected:
    RenderPriority m_render_priority{0};
    Graphite::Common::Utility::UniqueID m_view_id{};
    ApplicationPtr m_application{nullptr};
};

} // namespace Graphite::Application::Views
