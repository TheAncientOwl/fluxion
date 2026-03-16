/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TGraphiteApplication.hpp
/// @author Alexandru Delegeanu
/// @version 1.1
/// @brief Main application.
///

#pragma once

#include <algorithm>
#include <concepts>
#include <memory>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Core/Logger/Logger.hpp"

#include "ILayer.hpp"
#include "Renderer/Renderer.hpp"
#include "WindowConfiguration.hpp"

namespace Graphite::Core::Application {

template <typename ApplicationState>
class TGraphiteApplication
    : public Graphite::Core::Renderer::IRenderable
    , public std::enable_shared_from_this<TGraphiteApplication<ApplicationState>>
{
public:
    using Ptr = std::shared_ptr<TGraphiteApplication<ApplicationState>>;

    template <typename ApplicationImpl>
        requires std::derived_from<ApplicationImpl, TGraphiteApplication<ApplicationState>>
    static std::shared_ptr<ApplicationImpl> CreateApplication(
        WindowConfiguration window_configuration,
        ApplicationState app_state)
    {
        return std::shared_ptr<ApplicationImpl>(
            new ApplicationImpl(std::move(window_configuration), std::move(app_state)));
    }

    void Run();

    inline ApplicationState& GetApplicationState() noexcept;

    template <typename LayerImpl, typename... Args>
        requires std::derived_from<LayerImpl, ILayer<ApplicationState>> && requires {
            { LayerImpl::GetLayerName() } -> std::convertible_to<std::string_view>;
        }
    Graphite::Core::Common::UniqueID const& AddLayer(Args&&... args);

    void ActivateLayer(Graphite::Core::Common::UniqueID const& uid);
    void DeactivateLayer(Graphite::Core::Common::UniqueID const& uid);
    void RemoveLayer(Graphite::Core::Common::UniqueID const& uid);
    bool IsLayerPushed(Graphite::Core::Common::UniqueID const& uid) const;
    std::weak_ptr<ILayer<ApplicationState>> GetLayer(Graphite::Core::Common::UniqueID const& uid);
    std::weak_ptr<ILayer<ApplicationState>> const GetLayer(Graphite::Core::Common::UniqueID const& uid) const;

private:
    virtual void AppInit() = 0;

protected:
    TGraphiteApplication(WindowConfiguration window_configuration, ApplicationState initial_state);

private:
    void Init();
    void Render() override;
    void Shutdown();

    void RenderLayers();

protected:
    WindowConfiguration m_window_configuration{};
    ApplicationState m_app_state{};

private:
    std::vector<typename ILayer<ApplicationState>::Ptr> m_layers{};
    std::unordered_set<Graphite::Core::Common::UniqueID, Graphite::Core::Common::UniqueID::Hash>
        m_removed_layers{};
    std::unique_ptr<Graphite::Core::Renderer::IRenderer> m_renderer{nullptr};
};

template <typename ApplicationState>
TGraphiteApplication<ApplicationState>::TGraphiteApplication(
    WindowConfiguration window_configuration,
    ApplicationState initial_state)
    : m_window_configuration{std::move(window_configuration)}, m_app_state{std::move(initial_state)}
{
    LOG_SCOPE("");
}

template <typename ApplicationState>
void TGraphiteApplication<ApplicationState>::Run()
{
    LOG_SCOPE("");
    Init();
    m_renderer->Render(this->shared_from_this());
    Shutdown();
}

template <typename ApplicationState>
void TGraphiteApplication<ApplicationState>::Init()
{
    LOG_SCOPE("");
    m_renderer = Graphite::Core::Renderer::CreateRenderer();
    m_renderer->Init(m_window_configuration);

    AppInit();
}

template <typename ApplicationState>
void TGraphiteApplication<ApplicationState>::Render()
{
    LOG_SCOPE("");
    RenderLayers();
}

template <typename ApplicationState>
void TGraphiteApplication<ApplicationState>::Shutdown()
{
    LOG_SCOPE("");
    while (!m_layers.empty())
    {
        m_layers.back()->OnRemove();
        m_layers.pop_back();
    }
    m_renderer->Cleanup();
}

template <typename ApplicationState>
template <typename LayerImpl, typename... Args>
    requires std::derived_from<LayerImpl, ILayer<ApplicationState>> && requires {
        { LayerImpl::GetLayerName() } -> std::convertible_to<std::string_view>;
    }
Graphite::Core::Common::UniqueID const& TGraphiteApplication<ApplicationState>::AddLayer(Args&&... args)
{
    LOG_SCOPE("{}", LayerImpl::GetLayerName().data());
    auto layer = std::make_unique<LayerImpl>(std::forward<Args>(args)...);

    GRAPHITE_ASSERT(
        std::find_if(
            m_layers.cbegin(),
            m_layers.cend(),
            [&uid = layer->GetUID()](auto const& layer_ptr) { return layer_ptr->GetUID() == uid; }) ==
            m_layers.cend(),
        "Trying to add layer with same ID");

    layer->OnAdd();

    auto const& layer_uid{layer->GetUID()};
    m_layers.push_back(std::move(layer));

    std::sort(m_layers.begin(), m_layers.end(), [](auto const& a, auto const& b) {
        return a->GetZIndex() < b->GetZIndex();
    });

    return layer_uid;
}

template <typename ApplicationState>
void TGraphiteApplication<ApplicationState>::ActivateLayer(Graphite::Core::Common::UniqueID const& uid)
{
    auto it = std::find_if(m_layers.begin(), m_layers.end(), [uid](auto const& layer_ptr) {
        return layer_ptr->GetUID() == uid;
    });

    if (it != m_layers.end())
    {
        it->Activate();
    }
}

template <typename ApplicationState>
void TGraphiteApplication<ApplicationState>::DeactivateLayer(Graphite::Core::Common::UniqueID const& uid)
{
    auto it = std::find_if(m_layers.begin(), m_layers.end(), [uid](auto const& layer_ptr) {
        return layer_ptr->GetUID() == uid;
    });

    if (it != m_layers.end())
    {
        it->Deactivate();
    }
}

template <typename ApplicationState>
void TGraphiteApplication<ApplicationState>::RemoveLayer(Graphite::Core::Common::UniqueID const& uid)
{
    m_removed_layers.insert(uid);
}

template <typename ApplicationState>
bool TGraphiteApplication<ApplicationState>::IsLayerPushed(Graphite::Core::Common::UniqueID const& uid) const
{
    return std::find_if(m_layers.begin(), m_layers.end(), [uid](auto const& layer_ptr) {
               return layer_ptr->GetUID() == uid;
           }) != m_layers.end();
}

template <typename ApplicationState>
inline std::weak_ptr<ILayer<ApplicationState>> TGraphiteApplication<ApplicationState>::GetLayer(
    Graphite::Core::Common::UniqueID const& uid)
{
    auto it = std::find_if(m_layers.cbegin(), m_layers.cend(), [uid](auto const& layer_ptr) {
        return layer_ptr->GetUID() == uid;
    });

    return it != m_layers.cend() ? *it : nullptr;
}

template <typename ApplicationState>
inline ApplicationState& TGraphiteApplication<ApplicationState>::GetApplicationState() noexcept
{
    return m_app_state;
}

template <typename ApplicationState>
void TGraphiteApplication<ApplicationState>::RenderLayers()
{
    LOG_SCOPE("");
    m_removed_layers.clear();
    std::for_each(m_layers.begin(), m_layers.end(), [this](ILayer<ApplicationState>::Ptr& layer_ptr) {
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
                if (m_removed_layers.contains(layer_ptr->GetUID()))
                {
                    layer_ptr->OnRemove();
                    return true;
                }
                return false;
            }),
        m_layers.end());
}

} // namespace Graphite::Core::Application
