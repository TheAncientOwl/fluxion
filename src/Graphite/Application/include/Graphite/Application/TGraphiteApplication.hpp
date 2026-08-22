/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TGraphiteApplication.hpp
/// @author Alexandru Delegeanu
/// @version 1.10
/// @brief Main application.
///

#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Graphite/Logger.hpp"

#include "Graphite/Common/DataStructures/TThreadSafeQueue.hpp"
#include "Graphite/Common/Utility/TAppAction.hpp"
#include "Graphite/Renderer/Renderer.hpp"
#include "Views/TView.hpp"
#include "WindowConfiguration.hpp"

DEFINE_LOG_SCOPE(Graphite::Application::TGraphiteApplication);

namespace Graphite::Application {

template <typename ApplicationState, typename ActionEnum>
class TGraphiteApplication
    : public Graphite::Application::Renderer::IRenderable
    , public std::enable_shared_from_this<TGraphiteApplication<ApplicationState, ActionEnum>>
{
public: // Public Types
    using Ptr = std::shared_ptr<TGraphiteApplication<ApplicationState, ActionEnum>>;

public: // Public Static API
    template <typename ApplicationImpl>
        requires std::derived_from<ApplicationImpl, TGraphiteApplication<ApplicationState, ActionEnum>>
    static std::shared_ptr<ApplicationImpl> CreateApplication(
        WindowConfiguration window_configuration,
        ApplicationState app_state)
    {
        return std::shared_ptr<ApplicationImpl>(
            new ApplicationImpl(std::move(window_configuration), std::move(app_state)));
    }

public: // Public API
    void Run()
    {
        USE_LOG_SCOPE(Graphite::Application::TGraphiteApplication);
        LOG_SCOPE("::Run()");
        Init();
        m_renderer->Render(this->shared_from_this());
        Shutdown();
    }

    inline ApplicationState& GetApplicationState() noexcept { return m_app_state; }

    inline void PushAction(ActionEnum type, std::any&& payload = {})
    {
        m_action_queue.Push({type, std::move(payload)});
    }

    template <typename ViewImpl, typename... Args>
        requires std::derived_from<ViewImpl, Views::TView<ApplicationState, ActionEnum>> && requires {
            { ViewImpl::GetViewName() } -> std::convertible_to<std::string_view>;
        }
    Graphite::Common::Utility::UniqueID const& AddView(Args&&... args)
    {
        USE_LOG_SCOPE(Graphite::Application::TGraphiteApplication);
        LOG_SCOPE("::AddView(): {}", ViewImpl::GetViewName().data());
        auto view = std::make_unique<ViewImpl>(std::forward<Args>(args)...);

        GRAPHITE_ASSERT(
            std::find_if(
                m_views.cbegin(),
                m_views.cend(),
                [&id = view->GetID()](auto const& view_ptr) { return view_ptr->GetID() == id; }) ==
                m_views.cend(),
            "Trying to add view with same ID");

        view->OnAdd();

        auto const& view_id{view->GetID()};
        m_views.push_back(std::move(view));

        std::sort(m_views.begin(), m_views.end(), [](auto const& a, auto const& b) {
            return a->GetRenderPriority() < b->GetRenderPriority();
        });

        return view_id;
    }

    template <typename ViewType>
        requires std::is_class_v<ViewType> &&
                 std::derived_from<ViewType, Views::TView<ApplicationState, ActionEnum>>
    void ForEachView(std::function<void(ViewType&, bool const)> func)
    {
        USE_LOG_SCOPE(Graphite::Application::TGraphiteApplication);
        LOG_SCOPE("::ForEachView()");
        for (std::size_t idx = 0; idx < m_views.size(); ++idx)
        {
            auto& view{m_views[idx]};
            if (auto* casted = dynamic_cast<ViewType*>(view.get()))
            {
                LOG_TRACE("Applied to view ID {}", view->GetID());
                func(*casted, idx + 1 == m_views.size());
            }
        }
    }

    template <typename ApplicationImpl>
    std::shared_ptr<ApplicationImpl> As()
    {
        return std::static_pointer_cast<ApplicationImpl>(this->shared_from_this());
    }

    template <typename ApplicationImpl>
    std::shared_ptr<const ApplicationImpl> As() const
    {
        return std::static_pointer_cast<const ApplicationImpl>(this->shared_from_this());
    }

protected: // Shared API
    TGraphiteApplication(WindowConfiguration window_configuration, ApplicationState initial_state)
        : m_window_configuration{std::move(window_configuration)}
        , m_app_state{std::move(initial_state)}
    {
        USE_LOG_SCOPE(Graphite::Application::TGraphiteApplication);
        LOG_SCOPE("::TGraphiteApplication()");
    }

private: // Private API
    virtual void OnInit() = 0;
    virtual void OnShutdown() = 0;

    void Init()
    {
        USE_LOG_SCOPE(Graphite::Application::TGraphiteApplication);
        LOG_SCOPE("::Init()");
        m_renderer = Graphite::Application::Renderer::CreateRenderer();
        m_renderer->Init(m_window_configuration);

        m_worker_thread = std::thread(&TGraphiteApplication::WorkerLoop, this);

        OnInit();
    }

    void Render() override
    {
        USE_LOG_SCOPE(Graphite::Application::TGraphiteApplication);
        LOG_SCOPE("::Render()");
        IterateViews();
        RenderViews();
    }

    void Shutdown()
    {
        USE_LOG_SCOPE(Graphite::Application::TGraphiteApplication);
        LOG_SCOPE("::Shutdown()");

        OnShutdown();

        // 1. Cleanup Views
        while (!m_views.empty())
        {
            m_views.back()->OnRemove();
            m_views.pop_back();
        }

        // 1. Stop the worker thread gracefully
        m_worker_running.store(false);
        PushAction(static_cast<ActionEnum>(0)); // Push a dummy action to wake the CV

        if (m_worker_thread.joinable())
        {
            m_worker_thread.join();
        }

        m_renderer->Cleanup();
    }

    virtual void OnProcessAction(Graphite::Common::Utility::TAppAction<ActionEnum> const& action) = 0;

    void IterateViews()
    {
        USE_LOG_SCOPE(Graphite::Application::TGraphiteApplication);
        LOG_SCOPE("::IterateViews()");
        std::for_each(m_views.begin(), m_views.end(), [](auto& view_ptr) {
            if (view_ptr->IsActive())
            {
                view_ptr->OnIterate();
            }
        });
    }

    void RenderViews()
    {
        USE_LOG_SCOPE(Graphite::Application::TGraphiteApplication);
        LOG_SCOPE("::RenderViews()");
        m_removed_views.clear();
        std::for_each(
            m_views.begin(), m_views.end(), [](Views::TView<ApplicationState, ActionEnum>::Ptr& view_ptr) {
                if (view_ptr->IsActive())
                {
                    view_ptr->OnRender();
                }
            });

        m_views.erase(
            std::remove_if(
                m_views.begin(),
                m_views.end(),
                [&](auto& view_ptr) {
                    if (m_removed_views.contains(view_ptr->GetID()))
                    {
                        view_ptr->OnRemove();
                        return true;
                    }
                    return false;
                }),
            m_views.end());
    }

    void WorkerLoop()
    {
        Graphite::Common::Utility::TAppAction<ActionEnum> action;

        while (m_action_queue.WaitAndPop(action, m_worker_running))
        {
            USE_LOG_SCOPE(Graphite::Application::TGraphiteApplication);
            LOG_SCOPE("::WorkerLoop()");

            if (!m_worker_running.load())
                break;

            OnProcessAction(action);
        }
    }

protected: // Shared state
    WindowConfiguration m_window_configuration{};
    ApplicationState m_app_state{};

private: // Internal state
    std::vector<typename Views::TView<ApplicationState, ActionEnum>::Ptr> m_views{};
    std::unordered_set<Graphite::Common::Utility::UniqueID, Graphite::Common::Utility::UniqueID::Hash>
        m_removed_views{};
    std::unique_ptr<Graphite::Application::Renderer::IRenderer> m_renderer{nullptr};

private: // Threading conditions
    std::thread m_worker_thread{};
    std::atomic<bool> m_worker_running{true};
    Graphite::Common::DataStructures::TThreadSafeQueue<Graphite::Common::Utility::TAppAction<ActionEnum>>
        m_action_queue{};
};

} // namespace Graphite::Application
