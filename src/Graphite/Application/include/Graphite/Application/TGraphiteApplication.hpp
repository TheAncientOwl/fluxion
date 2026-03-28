/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TGraphiteApplication.hpp
/// @author Alexandru Delegeanu
/// @version 1.6
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

#include "Graphite/Renderer/Renderer.hpp"
#include "Layers/TLayer.hpp"
#include "TActionQueue.hpp"
#include "WindowConfiguration.hpp"

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
    void Run();

    inline ApplicationState& GetApplicationState() noexcept;

    inline void PushAction(ActionEnum type, std::any&& payload = {});

    template <typename LayerImpl, typename... Args>
        requires std::derived_from<LayerImpl, Layers::TLayer<ApplicationState, ActionEnum>> &&
                 requires {
                     { LayerImpl::GetLayerName() } -> std::convertible_to<std::string_view>;
                 }
    Graphite::Common::UniqueID const& AddLayer(Args&&... args);

    template <typename LayerType>
        requires std::is_class_v<LayerType> &&
                 std::derived_from<LayerType, Layers::TLayer<ApplicationState, ActionEnum>>
    void ForEachLayer(std::function<void(LayerType&, bool const)> func);

protected: // Shared API
    TGraphiteApplication(WindowConfiguration window_configuration, ApplicationState initial_state);

private: // Private API
    virtual void AppInit() = 0;

    void Init();
    void Render() override;
    void Shutdown();

    virtual void OnProcessAction(TAppAction<ActionEnum> const& action) = 0;

    void IterateLayers();
    void RenderLayers();
    void WorkerLoop();

protected: // Shared state
    WindowConfiguration m_window_configuration{};
    ApplicationState m_app_state{};

private: // Internal state
    std::vector<typename Layers::TLayer<ApplicationState, ActionEnum>::Ptr> m_layers{};
    std::unordered_set<Graphite::Common::UniqueID, Graphite::Common::UniqueID::Hash> m_removed_layers{};
    std::unique_ptr<Graphite::Application::Renderer::IRenderer> m_renderer{nullptr};

private: // Threading components
    std::thread m_worker_thread{};
    std::atomic<bool> m_worker_running{true};
    TThreadSafeQueue<TAppAction<ActionEnum>> m_action_queue{};
};

template <typename ApplicationState, typename ActionEnum>
TGraphiteApplication<ApplicationState, ActionEnum>::TGraphiteApplication(
    WindowConfiguration window_configuration,
    ApplicationState initial_state)
    : m_window_configuration{std::move(window_configuration)}, m_app_state{std::move(initial_state)}
{
    LOG_SCOPE("");
}

template <typename ApplicationState, typename ActionEnum>
void TGraphiteApplication<ApplicationState, ActionEnum>::Run()
{
    LOG_SCOPE("");
    Init();
    m_renderer->Render(this->shared_from_this());
    Shutdown();
}

template <typename ApplicationState, typename ActionEnum>
void TGraphiteApplication<ApplicationState, ActionEnum>::Init()
{
    LOG_SCOPE("");
    m_renderer = Graphite::Application::Renderer::CreateRenderer();
    m_renderer->Init(m_window_configuration);

    m_worker_thread = std::thread(&TGraphiteApplication::WorkerLoop, this);

    AppInit();
}

template <typename ApplicationState, typename ActionEnum>
void TGraphiteApplication<ApplicationState, ActionEnum>::Render()
{
    LOG_SCOPE("");
    IterateLayers();
    RenderLayers();
}

template <typename ApplicationState, typename ActionEnum>
void TGraphiteApplication<ApplicationState, ActionEnum>::Shutdown()
{
    LOG_SCOPE("");

    // 1. Cleanup Layers
    while (!m_layers.empty())
    {
        m_layers.back()->OnRemove();
        m_layers.pop_back();
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

template <typename ApplicationState, typename ActionEnum>
void TGraphiteApplication<ApplicationState, ActionEnum>::IterateLayers()
{
    LOG_SCOPE("");
    std::for_each(m_layers.begin(), m_layers.end(), [](auto& layer_ptr) {
        if (layer_ptr->IsActive())
        {
            layer_ptr->OnIterate();
        }
    });
}

template <typename ApplicationState, typename ActionEnum>
void TGraphiteApplication<ApplicationState, ActionEnum>::WorkerLoop()
{
    TAppAction<ActionEnum> action;

    while (m_action_queue.WaitAndPop(action, m_worker_running))
    {
        LOG_SCOPE("");

        if (!m_worker_running.load())
            break;

        OnProcessAction(action);
    }
}

template <typename ApplicationState, typename ActionEnum>
template <typename LayerImpl, typename... Args>
    requires std::derived_from<LayerImpl, Layers::TLayer<ApplicationState, ActionEnum>> && requires {
        { LayerImpl::GetLayerName() } -> std::convertible_to<std::string_view>;
    }
Graphite::Common::UniqueID const& TGraphiteApplication<ApplicationState, ActionEnum>::AddLayer(Args&&... args)
{
    LOG_SCOPE("{}", LayerImpl::GetLayerName().data());
    auto layer = std::make_unique<LayerImpl>(std::forward<Args>(args)...);

    GRAPHITE_ASSERT(
        std::find_if(
            m_layers.cbegin(),
            m_layers.cend(),
            [&id = layer->GetID()](auto const& layer_ptr) { return layer_ptr->GetID() == id; }) ==
            m_layers.cend(),
        "Trying to add layer with same ID");

    layer->OnAdd();

    auto const& layer_id{layer->GetID()};
    m_layers.push_back(std::move(layer));

    std::sort(m_layers.begin(), m_layers.end(), [](auto const& a, auto const& b) {
        return a->GetZIndex() < b->GetZIndex();
    });

    return layer_id;
}

template <typename ApplicationState, typename ActionEnum>
template <typename LayerType>
    requires std::is_class_v<LayerType> &&
             std::derived_from<LayerType, Layers::TLayer<ApplicationState, ActionEnum>>
inline void TGraphiteApplication<ApplicationState, ActionEnum>::ForEachLayer(
    std::function<void(LayerType&, bool const)> func)
{
    LOG_SCOPE("");
    for (std::size_t idx = 0; idx < m_layers.size(); ++idx)
    {
        auto& layer{m_layers[idx]};
        if (auto* casted = dynamic_cast<LayerType*>(layer.get()))
        {
            LOG_TRACE("Applied to layer ID {}", layer->GetID());
            func(*casted, idx + 1 == m_layers.size());
        }
    }
}

template <typename ApplicationState, typename ActionEnum>
inline ApplicationState& TGraphiteApplication<ApplicationState, ActionEnum>::GetApplicationState() noexcept
{
    return m_app_state;
}

template <typename ApplicationState, typename ActionEnum>
inline void TGraphiteApplication<ApplicationState, ActionEnum>::PushAction(ActionEnum type, std::any&& payload)
{
    m_action_queue.Push({type, std::move(payload)});
}

template <typename ApplicationState, typename ActionEnum>
void TGraphiteApplication<ApplicationState, ActionEnum>::RenderLayers()
{
    LOG_SCOPE("");
    m_removed_layers.clear();
    std::for_each(
        m_layers.begin(),
        m_layers.end(),
        [](Layers::TLayer<ApplicationState, ActionEnum>::Ptr& layer_ptr) {
            if (layer_ptr->IsActive())
            {
                layer_ptr->OnRender();
            }
        });

    m_layers.erase(
        std::remove_if(
            m_layers.begin(),
            m_layers.end(),
            [&](auto& layer_ptr) {
                if (m_removed_layers.contains(layer_ptr->GetID()))
                {
                    layer_ptr->OnRemove();
                    return true;
                }
                return false;
            }),
        m_layers.end());
}

} // namespace Graphite::Application
