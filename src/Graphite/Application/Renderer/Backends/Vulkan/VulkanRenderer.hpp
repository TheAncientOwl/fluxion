/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file VulkanRenderer.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Vulkan implementation of @see Graphite/Renderer.hpp.
/// Adaptation of @see external/imgui/examples/example_glfw_Vulkan/main.cpp
///

#pragma once

#include "Graphite/Application/Renderer/Renderer.hpp"

#ifdef GRAPHITE_USE_VULKAN_API
#include <vulkan/vulkan.h>
#include "backends/imgui_impl_vulkan.h"
// Forward-declare GLFW types here to avoid including glfw3.h before
// compilation units can define GLFW_INCLUDE_VULKAN as needed.
struct GLFWwindow;
#endif

namespace Graphite::Application::Renderer::Backends::Vulkan {

class VulkanRenderer : public IRenderer
{
public:
    VulkanRenderer();
    ~VulkanRenderer() override;

    void Init(Graphite::Application::WindowConfiguration const& window_configuration) override;
    void Render(std::shared_ptr<IRenderable> user_interface) override;
    void Cleanup() override;

    struct State
    {
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
#endif
        VkAllocationCallbacks* allocator = nullptr;
        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        uint32_t queueFamily = (uint32_t)-1;
        VkQueue queue = VK_NULL_HANDLE;
        VkPipelineCache pipelineCache = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        ImGui_ImplVulkanH_Window mainWindowData{};
        uint32_t minImageCount = 2;
        bool swapChainRebuild = false;
        GLFWwindow* window = nullptr;
    };

private:
    State m_state;
    bool m_cleaned_up{false};
};

} // namespace Graphite::Application::Renderer::Backends::Vulkan
