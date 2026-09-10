/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file VulkanRenderer.cpp
/// @author Alexandru Delegeanu
/// @version 1.10
/// @brief Implementation of @see VulkanRenderer.hpp.
///

#include "VulkanRenderer.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Graphite::Application::Renderer::Vulkan);
USE_LOG_SCOPE(Graphite::Application::Renderer::Vulkan);

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <dwmapi.h>
#include <windows.h>
#endif

#include <GLFW/glfw3.h>

#if defined(_WIN32)
#include <GLFW/glfw3native.h>
#endif

// Volk headers
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

namespace {

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static const char* VkResultToString(VkResult result)
{
    switch (result)
    {
    case VK_SUCCESS:
        return "VK_SUCCESS";
    case VK_NOT_READY:
        return "VK_NOT_READY";
    case VK_TIMEOUT:
        return "VK_TIMEOUT";
    case VK_EVENT_SET:
        return "VK_EVENT_SET";
    case VK_EVENT_RESET:
        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
        return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:
        return "VK_ERROR_VALIDATION_FAILED_EXT";
    default:
        return "UNKNOWN_VULKAN_ERROR";
    }
}

static void check_vk_result(VkResult err)
{
    if (err == VK_SUCCESS)
        return;

    fprintf(
        stderr, "[vulkan] Error: VkResult = %d (%s)\n", static_cast<int>(err), VkResultToString(err));

    if (err < 0)
        abort();
}

static bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension)
{
    for (const VkExtensionProperties& p : properties)
    {
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    }

    return false;
}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static bool IsLayerAvailable(const ImVector<VkLayerProperties>& properties, const char* layer)
{
    for (const VkLayerProperties& p : properties)
    {
        if (strcmp(p.layerName, layer) == 0)
            return true;
    }

    return false;
}
#endif

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char* pLayerPrefix,
    const char* pMessage,
    void* pUserData)
{
    (void)flags;
    (void)object;
    (void)location;
    (void)messageCode;
    (void)pUserData;
    (void)pLayerPrefix;

    LOG_DEBUG("[vulkan] Debug report from ObjectType: {} Message: {}", objectType, pMessage);

    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);

    return VK_FALSE;
}

#endif // APP_USE_VULKAN_DEBUG_REPORT

static uint32_t FindQueueFamily(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    uint32_t queue_family_count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    if (queue_family_count == 0)
        return UINT32_MAX;

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);

    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < queue_family_count; ++i)
    {
        const VkQueueFamilyProperties& family = queue_families[i];

        if ((family.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
            continue;

        VkBool32 present_supported = VK_FALSE;

        VkResult result =
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_supported);

        if (result != VK_SUCCESS)
        {
            fprintf(
                stderr,
                "[vulkan] vkGetPhysicalDeviceSurfaceSupportKHR failed for queue family %u: %s\n",
                i,
                VkResultToString(result));

            continue;
        }

        if (present_supported == VK_TRUE)
            return i;
    }

    return UINT32_MAX;
}

static VkPhysicalDevice SelectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t device_count = 0;

    VkResult result = vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (result != VK_SUCCESS || device_count == 0)
    {
        fprintf(
            stderr, "[vulkan] No Vulkan physical devices found. Result: %s\n", VkResultToString(result));

        throw std::runtime_error("No Vulkan physical device was found.");
    }

    std::vector<VkPhysicalDevice> devices(device_count);

    result = vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkEnumeratePhysicalDevices failed: %s\n", VkResultToString(result));

        throw std::runtime_error("Failed to enumerate Vulkan physical devices.");
    }

    VkPhysicalDevice discrete_gpu = VK_NULL_HANDLE;
    VkPhysicalDevice integrated_gpu = VK_NULL_HANDLE;
    VkPhysicalDevice other_gpu = VK_NULL_HANDLE;

    for (VkPhysicalDevice device : devices)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device, &properties);

        uint32_t queue_family = FindQueueFamily(device, surface);

        if (queue_family == UINT32_MAX)
            continue;

        fprintf(stderr, "[vulkan] Found compatible GPU: %s\n", properties.deviceName);

        fprintf(
            stderr,
            "[vulkan]   API version: %u.%u.%u\n",
            VK_VERSION_MAJOR(properties.apiVersion),
            VK_VERSION_MINOR(properties.apiVersion),
            VK_VERSION_PATCH(properties.apiVersion));

        fprintf(stderr, "[vulkan]   Driver version: %u\n", properties.driverVersion);

        fprintf(stderr, "[vulkan]   Queue family: %u\n", queue_family);

        switch (properties.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            if (discrete_gpu == VK_NULL_HANDLE)
                discrete_gpu = device;
            break;

        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            if (integrated_gpu == VK_NULL_HANDLE)
                integrated_gpu = device;
            break;

        default:
            if (other_gpu == VK_NULL_HANDLE)
                other_gpu = device;
            break;
        }
    }

    if (discrete_gpu != VK_NULL_HANDLE)
        return discrete_gpu;

    if (integrated_gpu != VK_NULL_HANDLE)
        return integrated_gpu;

    if (other_gpu != VK_NULL_HANDLE)
        return other_gpu;

    throw std::runtime_error(
        "No Vulkan physical device with graphics/presentation support was found.");
}

static void SetupVulkan(
    ::Graphite::Application::Renderer::Backends::Vulkan::VulkanRenderer::State& state,
    const ImVector<const char*>& glfw_extensions)
{
    VkResult err;

#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
    err = volkInitialize();

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] volkInitialize failed: %s\n", VkResultToString(err));

        throw std::runtime_error("Failed to initialize Vulkan loader.");
    }
#endif

    // ----------------------------------------------------------------------
    // Enumerate instance extensions
    // ----------------------------------------------------------------------

    uint32_t properties_count = 0;

    err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);

    if (err != VK_SUCCESS)
    {
        fprintf(
            stderr, "[vulkan] Failed to enumerate instance extensions: %s\n", VkResultToString(err));

        throw std::runtime_error("Failed to enumerate Vulkan instance extensions.");
    }

    ImVector<VkExtensionProperties> properties;
    properties.resize(static_cast<int>(properties_count));

    err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.Data);

    if (err != VK_SUCCESS)
    {
        fprintf(
            stderr, "[vulkan] Failed to enumerate instance extensions: %s\n", VkResultToString(err));

        throw std::runtime_error("Failed to enumerate Vulkan instance extensions.");
    }

    // ----------------------------------------------------------------------
    // Build instance extension list
    //
    // IMPORTANT:
    // GLFW owns the WSI requirements. Do NOT manually add VK_KHR_surface or
    // VK_KHR_win32_surface here.
    // ----------------------------------------------------------------------

    ImVector<const char*> instance_extensions;

    for (int i = 0; i < glfw_extensions.Size; ++i)
    {
        const char* extension = glfw_extensions[i];

        if (!IsExtensionAvailable(properties, extension))
        {
            fprintf(stderr, "[vulkan] GLFW requested unavailable instance extension: %s\n", extension);

            throw std::runtime_error(
                std::string("Required GLFW Vulkan extension is unavailable: ") + extension);
        }

        instance_extensions.push_back(extension);
    }

    // Optional extension.
    if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
    {
        bool already_enabled = false;

        for (int i = 0; i < instance_extensions.Size; ++i)
        {
            if (strcmp(instance_extensions[i], VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) ==
                0)
            {
                already_enabled = true;
                break;
            }
        }

        if (!already_enabled)
        {
            instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }
    }

#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    bool portability_enumeration = false;

    if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
    {
        instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        portability_enumeration = true;
    }
#endif

    // ----------------------------------------------------------------------
    // Validation layers
    // ----------------------------------------------------------------------

    uint32_t layer_count = 0;

    err = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] Failed to enumerate Vulkan layers: %s\n", VkResultToString(err));
    }

    ImVector<VkLayerProperties> layers;

    if (layer_count > 0)
    {
        layers.resize(static_cast<int>(layer_count));

        err = vkEnumerateInstanceLayerProperties(&layer_count, layers.Data);

        if (err != VK_SUCCESS)
        {
            fprintf(stderr, "[vulkan] Failed to enumerate Vulkan layers: %s\n", VkResultToString(err));
        }
    }

    // ----------------------------------------------------------------------
    // Create Vulkan instance
    // ----------------------------------------------------------------------

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Fluxion";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Fluxion";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    if (portability_enumeration)
    {
        create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

#ifdef APP_USE_VULKAN_DEBUG_REPORT

    const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";

    bool validation_available = IsLayerAvailable(layers, validation_layer_name);

    if (validation_available)
    {
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = &validation_layer_name;

        if (IsExtensionAvailable(properties, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        {
            instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
    }
    else
    {
        fprintf(stderr, "[vulkan] Validation layer unavailable; continuing without validation.\n");
    }

#endif

    create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.Size);

    create_info.ppEnabledExtensionNames = instance_extensions.Data;

    fprintf(stderr, "[vulkan] Creating Vulkan instance...\n");

    err = vkCreateInstance(&create_info, state.allocator, &state.instance);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkCreateInstance failed: %s\n", VkResultToString(err));

        throw std::runtime_error("Failed to create Vulkan instance.");
    }

#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
    volkLoadInstance(state.instance);
#endif

    fprintf(stderr, "[vulkan] Vulkan instance created successfully.\n");

#ifdef APP_USE_VULKAN_DEBUG_REPORT

    if (validation_available && IsExtensionAvailable(properties, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
    {
        auto f_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(state.instance, "vkCreateDebugReportCallbackEXT"));

        if (f_vkCreateDebugReportCallbackEXT != nullptr)
        {
            VkDebugReportCallbackCreateInfoEXT debug_report_ci{};
            debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;

            debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

            debug_report_ci.pfnCallback = debug_report;

            err = f_vkCreateDebugReportCallbackEXT(
                state.instance, &debug_report_ci, state.allocator, &state.debugReport);

            if (err != VK_SUCCESS)
            {
                fprintf(
                    stderr, "[vulkan] Failed to create debug callback: %s\n", VkResultToString(err));
            }
        }
    }

#endif
}

static void SetupVulkanDevice(
    ::Graphite::Application::Renderer::Backends::Vulkan::VulkanRenderer::State& state)
{
    // ----------------------------------------------------------------------
    // Select physical device
    // ----------------------------------------------------------------------

    state.physicalDevice = SelectPhysicalDevice(state.instance, state.mainWindowData.Surface);

    if (state.physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to select Vulkan physical device.");
    }

    VkPhysicalDeviceProperties gpu_properties{};

    vkGetPhysicalDeviceProperties(state.physicalDevice, &gpu_properties);

    fprintf(stderr, "[vulkan] Selected GPU: %s\n", gpu_properties.deviceName);

    // ----------------------------------------------------------------------
    // Select graphics + presentation queue
    // ----------------------------------------------------------------------

    state.queueFamily = FindQueueFamily(state.physicalDevice, state.mainWindowData.Surface);

    if (state.queueFamily == UINT32_MAX)
    {
        throw std::runtime_error(
            "Could not find Vulkan queue family supporting graphics and presentation.");
    }

    fprintf(stderr, "[vulkan] Selected queue family: %u\n", state.queueFamily);

    // ----------------------------------------------------------------------
    // Check device extensions
    // ----------------------------------------------------------------------

    uint32_t properties_count = 0;

    VkResult err = vkEnumerateDeviceExtensionProperties(
        state.physicalDevice, nullptr, &properties_count, nullptr);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] Failed to enumerate device extensions: %s\n", VkResultToString(err));

        throw std::runtime_error("Failed to enumerate Vulkan device extensions.");
    }

    ImVector<VkExtensionProperties> properties;
    properties.resize(static_cast<int>(properties_count));

    err = vkEnumerateDeviceExtensionProperties(
        state.physicalDevice, nullptr, &properties_count, properties.Data);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] Failed to enumerate device extensions: %s\n", VkResultToString(err));

        throw std::runtime_error("Failed to enumerate Vulkan device extensions.");
    }

    if (!IsExtensionAvailable(properties, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
    {
        throw std::runtime_error("Selected Vulkan device does not support VK_KHR_swapchain.");
    }

    ImVector<const char*> device_extensions;

    device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
    {
        device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    }
#endif

    // ----------------------------------------------------------------------
    // Create logical device
    // ----------------------------------------------------------------------

    const float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_info{};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

    queue_info.queueFamilyIndex = state.queueFamily;

    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_info;

    device_create_info.pEnabledFeatures = &device_features;

    device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.Size);

    device_create_info.ppEnabledExtensionNames = device_extensions.Data;

    fprintf(stderr, "[vulkan] Creating logical device...\n");

    err = vkCreateDevice(state.physicalDevice, &device_create_info, state.allocator, &state.device);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkCreateDevice failed: %s\n", VkResultToString(err));

        throw std::runtime_error("Failed to create Vulkan logical device.");
    }

    vkGetDeviceQueue(state.device, state.queueFamily, 0, &state.queue);

    if (state.queue == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to retrieve Vulkan graphics queue.");
    }

    fprintf(stderr, "[vulkan] Logical device created successfully.\n");

    // ----------------------------------------------------------------------
    // Descriptor pool
    // ----------------------------------------------------------------------

    VkDescriptorPoolSize pool_sizes[] = {
        {
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
        },
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    pool_info.maxSets = 0;

    for (VkDescriptorPoolSize& pool_size : pool_sizes)
        pool_info.maxSets += pool_size.descriptorCount;

    pool_info.poolSizeCount = static_cast<uint32_t>(IM_COUNTOF(pool_sizes));

    pool_info.pPoolSizes = pool_sizes;

    err = vkCreateDescriptorPool(state.device, &pool_info, state.allocator, &state.descriptorPool);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkCreateDescriptorPool failed: %s\n", VkResultToString(err));

        throw std::runtime_error("Failed to create Vulkan descriptor pool.");
    }
}

static void SetupVulkanWindow(
    ::Graphite::Application::Renderer::Backends::Vulkan::VulkanRenderer::State& state,
    ImGui_ImplVulkanH_Window* wd,
    VkSurfaceKHR surface,
    int width,
    int height)
{
    VkBool32 supported = VK_FALSE;

    VkResult err = vkGetPhysicalDeviceSurfaceSupportKHR(
        state.physicalDevice, state.queueFamily, surface, &supported);

    if (err != VK_SUCCESS)
    {
        fprintf(
            stderr, "[vulkan] vkGetPhysicalDeviceSurfaceSupportKHR failed: %s\n", VkResultToString(err));

        throw std::runtime_error("Failed to query Vulkan presentation support.");
    }

    if (supported != VK_TRUE)
    {
        fprintf(stderr, "[vulkan] Selected queue family does not support presentation.\n");

        throw std::runtime_error("Selected Vulkan queue family does not support presentation.");
    }

    // ----------------------------------------------------------------------
    // Surface format
    // ----------------------------------------------------------------------

    const VkFormat request_surface_image_format[] = {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8_UNORM,
        VK_FORMAT_R8G8B8_UNORM,
    };

    const VkColorSpaceKHR request_surface_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    wd->Surface = surface;

    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
        state.physicalDevice,
        wd->Surface,
        request_surface_image_format,
        IM_COUNTOF(request_surface_image_format),
        request_surface_color_space);

    // ----------------------------------------------------------------------
    // Present mode
    // ----------------------------------------------------------------------

#ifdef APP_USE_UNLIMITED_FRAME_RATE

    VkPresentModeKHR present_modes[] = {
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_PRESENT_MODE_FIFO_KHR,
    };

#else

    VkPresentModeKHR present_modes[] = {
        VK_PRESENT_MODE_FIFO_KHR,
    };

#endif

    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
        state.physicalDevice, wd->Surface, present_modes, IM_COUNTOF(present_modes));

    fprintf(stderr, "[vulkan] Creating swapchain/window: %dx%d\n", width, height);

    // ----------------------------------------------------------------------
    // Swapchain
    // ----------------------------------------------------------------------

    if (state.minImageCount < 2)
        state.minImageCount = 2;

    ImGui_ImplVulkanH_CreateOrResizeWindow(
        state.instance,
        state.physicalDevice,
        state.device,
        wd,
        state.queueFamily,
        state.allocator,
        width,
        height,
        state.minImageCount,
        0);

    // Explicitly initialize clear color.
    wd->ClearValue.color.float32[0] = 0.10f;
    wd->ClearValue.color.float32[1] = 0.10f;
    wd->ClearValue.color.float32[2] = 0.10f;
    wd->ClearValue.color.float32[3] = 1.0f;

    fprintf(
        stderr, "[vulkan] Swapchain created: %ux%u, images=%u\n", wd->Width, wd->Height, wd->ImageCount);
}

static void CleanupVulkanWindow(
    ::Graphite::Application::Renderer::Backends::Vulkan::VulkanRenderer::State& state,
    ImGui_ImplVulkanH_Window* wd)
{
    if (state.device == VK_NULL_HANDLE)
        return;

    if (wd->Surface != VK_NULL_HANDLE)
    {
        ImGui_ImplVulkanH_DestroyWindow(state.instance, state.device, wd, state.allocator);

        wd->Surface = VK_NULL_HANDLE;
    }
}

static void CleanupVulkan(::Graphite::Application::Renderer::Backends::Vulkan::VulkanRenderer::State& state)
{
    if (state.device != VK_NULL_HANDLE)
    {
        if (state.descriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(state.device, state.descriptorPool, state.allocator);

            state.descriptorPool = VK_NULL_HANDLE;
        }

#ifdef APP_USE_VULKAN_DEBUG_REPORT

        if (state.debugReport != VK_NULL_HANDLE)
        {
            auto f_vkDestroyDebugReportCallbackEXT =
                reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
                    vkGetInstanceProcAddr(state.instance, "vkDestroyDebugReportCallbackEXT"));

            if (f_vkDestroyDebugReportCallbackEXT != nullptr)
            {
                f_vkDestroyDebugReportCallbackEXT(state.instance, state.debugReport, state.allocator);
            }

            state.debugReport = VK_NULL_HANDLE;
        }

#endif

        vkDestroyDevice(state.device, state.allocator);

        state.device = VK_NULL_HANDLE;
    }

    if (state.instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(state.instance, state.allocator);

        state.instance = VK_NULL_HANDLE;
    }
}

static void FrameRender(
    ::Graphite::Application::Renderer::Backends::Vulkan::VulkanRenderer::State& state,
    ImGui_ImplVulkanH_Window* wd,
    ImDrawData* draw_data)
{
    VkSemaphore image_acquired_semaphore =
        wd->FrameSemaphores[static_cast<int>(wd->SemaphoreIndex)].ImageAcquiredSemaphore;

    VkSemaphore render_complete_semaphore =
        wd->FrameSemaphores[static_cast<int>(wd->SemaphoreIndex)].RenderCompleteSemaphore;

    VkResult err = vkAcquireNextImageKHR(
        state.device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);

    if (err == VK_ERROR_OUT_OF_DATE_KHR)
    {
        state.swapChainRebuild = true;
        return;
    }

    if (err == VK_SUBOPTIMAL_KHR)
    {
        state.swapChainRebuild = true;
    }
    else if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkAcquireNextImageKHR failed: %s\n", VkResultToString(err));

        check_vk_result(err);
        return;
    }

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[static_cast<int>(wd->FrameIndex)];

    // ----------------------------------------------------------------------
    // Wait for previous frame
    // ----------------------------------------------------------------------

    err = vkWaitForFences(state.device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkWaitForFences failed: %s\n", VkResultToString(err));

        check_vk_result(err);
        return;
    }

    err = vkResetFences(state.device, 1, &fd->Fence);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkResetFences failed: %s\n", VkResultToString(err));

        check_vk_result(err);
        return;
    }

    // ----------------------------------------------------------------------
    // Reset command buffer
    // ----------------------------------------------------------------------

    err = vkResetCommandPool(state.device, fd->CommandPool, 0);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkResetCommandPool failed: %s\n", VkResultToString(err));

        check_vk_result(err);
        return;
    }

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    err = vkBeginCommandBuffer(fd->CommandBuffer, &begin_info);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkBeginCommandBuffer failed: %s\n", VkResultToString(err));

        check_vk_result(err);
        return;
    }

    // ----------------------------------------------------------------------
    // Render pass
    // ----------------------------------------------------------------------

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    render_pass_info.renderPass = wd->RenderPass;

    render_pass_info.framebuffer = fd->Framebuffer;

    render_pass_info.renderArea.extent.width = static_cast<uint32_t>(wd->Width);

    render_pass_info.renderArea.extent.height = static_cast<uint32_t>(wd->Height);

    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &wd->ClearValue;

    vkCmdBeginRenderPass(fd->CommandBuffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    // ----------------------------------------------------------------------
    // ImGui rendering
    // ----------------------------------------------------------------------

    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    vkCmdEndRenderPass(fd->CommandBuffer);

    // ----------------------------------------------------------------------
    // Submit
    // ----------------------------------------------------------------------

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_acquired_semaphore;

    submit_info.pWaitDstStageMask = &wait_stage;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &fd->CommandBuffer;

    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &render_complete_semaphore;

    err = vkEndCommandBuffer(fd->CommandBuffer);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkEndCommandBuffer failed: %s\n", VkResultToString(err));

        check_vk_result(err);
        return;
    }

    err = vkQueueSubmit(state.queue, 1, &submit_info, fd->Fence);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkQueueSubmit failed: %s\n", VkResultToString(err));

        check_vk_result(err);
        return;
    }
}

static void FramePresent(
    ::Graphite::Application::Renderer::Backends::Vulkan::VulkanRenderer::State& state,
    ImGui_ImplVulkanH_Window* wd)
{
    if (state.swapChainRebuild)
        return;

    VkSemaphore render_complete_semaphore =
        wd->FrameSemaphores[static_cast<int>(wd->SemaphoreIndex)].RenderCompleteSemaphore;

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_complete_semaphore;

    present_info.swapchainCount = 1;
    present_info.pSwapchains = &wd->Swapchain;

    present_info.pImageIndices = &wd->FrameIndex;

    VkResult err = vkQueuePresentKHR(state.queue, &present_info);

    if (err == VK_ERROR_OUT_OF_DATE_KHR)
    {
        state.swapChainRebuild = true;
        return;
    }

    if (err == VK_SUBOPTIMAL_KHR)
    {
        state.swapChainRebuild = true;
    }
    else if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] vkQueuePresentKHR failed: %s\n", VkResultToString(err));

        check_vk_result(err);
        return;
    }

    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount;
}

} // anonymous namespace

namespace Graphite::Application::Renderer::Backends::Vulkan {

VulkanRenderer::VulkanRenderer()
{
    LOG_SCOPE("::VulkanRenderer()");
}

VulkanRenderer::~VulkanRenderer()
{
    LOG_SCOPE("::~VulkanRenderer()");
    Cleanup();
}

void VulkanRenderer::Init(Graphite::Application::WindowConfiguration const& window_configuration)
{
    LOG_SCOPE("::Init()");

    // ----------------------------------------------------------------------
    // GLFW
    // ----------------------------------------------------------------------

    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
    {
        const std::string msg =
            "::Graphite::Application::Renderer::Backends::Vulkan::VulkanRenderer(): "
            "[Critical] could not initialize GLFW";

        std::cerr << msg << std::endl;

        throw std::runtime_error(msg);
    }

    if (!glfwVulkanSupported())
    {
        const std::string msg =
            "GLFW reports that Vulkan is not supported on this machine. "
            "Please install/update the Vulkan runtime/graphics driver and try again.";

        std::cerr << msg << std::endl;

        glfwTerminate();

        throw std::runtime_error(msg);
    }

    // ----------------------------------------------------------------------
    // GLFW window
    //
    // IMPORTANT:
    // GLFW window dimensions are logical dimensions.
    // Do not multiply them by the monitor DPI scale.
    //
    // Vulkan uses glfwGetFramebufferSize() below to obtain the actual
    // framebuffer dimensions.
    // ----------------------------------------------------------------------

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    int target_width =
        window_configuration.width > 0 ? static_cast<int>(window_configuration.width) : 950;

    int target_height =
        window_configuration.height > 0 ? static_cast<int>(window_configuration.height) : 750;

    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();

    float main_scale = 1.0f;

    if (primary_monitor != nullptr)
    {
        main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(primary_monitor);

        if (main_scale <= 0.0f)
            main_scale = 1.0f;
    }

    fprintf(
        stderr,
        "[glfw] Creating window: %dx%d, content scale: %.2f\n",
        target_width,
        target_height,
        static_cast<double>(main_scale));

    m_state.window = glfwCreateWindow(
        target_width, target_height, window_configuration.title.c_str(), nullptr, nullptr);

    if (m_state.window == nullptr)
    {
        const std::string msg = "Failed to create a GLFW window for the Vulkan renderer.";

        std::cerr << msg << std::endl;

        glfwTerminate();

        throw std::runtime_error(msg);
    }

#if defined(_WIN32)

    HWND const window_handle = glfwGetWin32Window(m_state.window);

    HICON const icon = static_cast<HICON>(LoadImageW(
        GetModuleHandleW(nullptr), MAKEINTRESOURCEW(101), IMAGE_ICON, 32, 32, LR_DEFAULTSIZE));

    HICON const small_icon = static_cast<HICON>(LoadImageW(
        GetModuleHandleW(nullptr), MAKEINTRESOURCEW(101), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE));

    if (icon != nullptr)
    {
        SendMessageW(window_handle, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
    }

    if (small_icon != nullptr)
    {
        SendMessageW(window_handle, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(small_icon));
    }

    const COLORREF caption_color = RGB(45, 45, 48);

    const COLORREF text_color = RGB(241, 241, 241);

    DwmSetWindowAttribute(window_handle, DWMWA_CAPTION_COLOR, &caption_color, sizeof(caption_color));

    DwmSetWindowAttribute(window_handle, DWMWA_TEXT_COLOR, &text_color, sizeof(text_color));

#endif

    // ----------------------------------------------------------------------
    // GLFW Vulkan extensions
    // ----------------------------------------------------------------------

    uint32_t extensions_count = 0;

    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);

    if (glfw_extensions == nullptr || extensions_count == 0)
    {
        glfwDestroyWindow(m_state.window);
        m_state.window = nullptr;

        glfwTerminate();

        throw std::runtime_error("GLFW did not provide the required Vulkan instance extensions.");
    }

    ImVector<const char*> extensions;

    for (uint32_t i = 0; i < extensions_count; ++i)
    {
        fprintf(stderr, "[vulkan] GLFW required extension: %s\n", glfw_extensions[i]);

        extensions.push_back(glfw_extensions[i]);
    }

    // ----------------------------------------------------------------------
    // Create Vulkan instance
    // ----------------------------------------------------------------------

    SetupVulkan(m_state, extensions);

    // ----------------------------------------------------------------------
    // Create GLFW Vulkan surface BEFORE selecting physical device.
    //
    // This lets us choose a GPU/queue family that can actually present to
    // this Windows surface.
    // ----------------------------------------------------------------------

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkResult err =
        glfwCreateWindowSurface(m_state.instance, m_state.window, m_state.allocator, &surface);

    if (err != VK_SUCCESS)
    {
        fprintf(stderr, "[vulkan] glfwCreateWindowSurface failed: %s\n", VkResultToString(err));

        CleanupVulkan(m_state);

        glfwDestroyWindow(m_state.window);
        m_state.window = nullptr;

        glfwTerminate();

        throw std::runtime_error("Failed to create GLFW Vulkan surface.");
    }

    m_state.mainWindowData.Surface = surface;

    fprintf(stderr, "[vulkan] GLFW Vulkan surface created successfully.\n");

    // ----------------------------------------------------------------------
    // Select physical device and create logical device.
    // ----------------------------------------------------------------------

    SetupVulkanDevice(m_state);

    // ----------------------------------------------------------------------
    // Get framebuffer dimensions.
    //
    // On Windows with DPI scaling, these can differ from glfwGetWindowSize().
    // This is the size Vulkan should use.
    // ----------------------------------------------------------------------

    int window_width = 0;
    int window_height = 0;

    glfwGetWindowSize(m_state.window, &window_width, &window_height);

    int framebuffer_width = 0;
    int framebuffer_height = 0;

    glfwGetFramebufferSize(m_state.window, &framebuffer_width, &framebuffer_height);

    fprintf(stderr, "[glfw] Window size: %dx%d\n", window_width, window_height);

    fprintf(stderr, "[glfw] Framebuffer size: %dx%d\n", framebuffer_width, framebuffer_height);

    if (framebuffer_width <= 0 || framebuffer_height <= 0)
    {
        framebuffer_width = target_width;
        framebuffer_height = target_height;
    }

    // ----------------------------------------------------------------------
    // Create swapchain/window
    // ----------------------------------------------------------------------

    ImGui_ImplVulkanH_Window* wd = &m_state.mainWindowData;

    SetupVulkanWindow(m_state, wd, surface, framebuffer_width, framebuffer_height);

    // ----------------------------------------------------------------------
    // Dear ImGui
    // ----------------------------------------------------------------------

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // ----------------------------------------------------------------------
    // ImGui scaling
    // ----------------------------------------------------------------------

    ImGuiStyle& style = ImGui::GetStyle();

    style.ScaleAllSizes(main_scale);

    style.FontScaleDpi = main_scale;

    io.ConfigDpiScaleFonts = true;

    io.ConfigDpiScaleViewports = true;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // ----------------------------------------------------------------------
    // ImGui GLFW backend
    // ----------------------------------------------------------------------

    ImGui_ImplGlfw_InitForVulkan(m_state.window, true);

    // ----------------------------------------------------------------------
    // ImGui Vulkan backend
    // ----------------------------------------------------------------------

    ImGui_ImplVulkan_InitInfo init_info{};

    init_info.Instance = m_state.instance;

    init_info.PhysicalDevice = m_state.physicalDevice;

    init_info.Device = m_state.device;

    init_info.QueueFamily = m_state.queueFamily;

    init_info.Queue = m_state.queue;

    init_info.PipelineCache = m_state.pipelineCache;

    init_info.DescriptorPool = m_state.descriptorPool;

    init_info.MinImageCount = m_state.minImageCount;

    init_info.ImageCount = wd->ImageCount;

    init_info.Allocator = m_state.allocator;

    init_info.PipelineInfoMain.RenderPass = wd->RenderPass;

    init_info.PipelineInfoMain.Subpass = 0;

    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    init_info.CheckVkResultFn = check_vk_result;

    ImGui_ImplVulkan_Init(&init_info);

    fprintf(stderr, "[vulkan] ImGui Vulkan backend initialized successfully.\n");

    fprintf(stderr, "[vulkan] Initialization complete.\n");
}

void VulkanRenderer::Render(std::shared_ptr<IRenderable> user_interface)
{
    LOG_SCOPE("Main Render Loop");

    ImGui_ImplVulkanH_Window* wd = &m_state.mainWindowData;

    while (!glfwWindowShouldClose(m_state.window))
    {
        // ------------------------------------------------------------------
        // Poll events
        // ------------------------------------------------------------------

        glfwPollEvents();

        // ------------------------------------------------------------------
        // Detect framebuffer resize
        // ------------------------------------------------------------------

        int fb_width = 0;
        int fb_height = 0;

        glfwGetFramebufferSize(m_state.window, &fb_width, &fb_height);

        if (fb_width > 0 && fb_height > 0 &&
            (m_state.swapChainRebuild || m_state.mainWindowData.Width != fb_width ||
             m_state.mainWindowData.Height != fb_height))
        {
            fprintf(stderr, "[vulkan] Recreating swapchain: %dx%d\n", fb_width, fb_height);

            VkResult wait_result = vkDeviceWaitIdle(m_state.device);

            if (wait_result != VK_SUCCESS)
            {
                fprintf(
                    stderr,
                    "[vulkan] vkDeviceWaitIdle before resize failed: %s\n",
                    VkResultToString(wait_result));
            }

            ImGui_ImplVulkan_SetMinImageCount(m_state.minImageCount);

            ImGui_ImplVulkanH_CreateOrResizeWindow(
                m_state.instance,
                m_state.physicalDevice,
                m_state.device,
                wd,
                m_state.queueFamily,
                m_state.allocator,
                fb_width,
                fb_height,
                m_state.minImageCount,
                0);

            wd->ClearValue.color.float32[0] = 0.10f;
            wd->ClearValue.color.float32[1] = 0.10f;
            wd->ClearValue.color.float32[2] = 0.10f;
            wd->ClearValue.color.float32[3] = 1.0f;

            m_state.mainWindowData.FrameIndex = 0;
            m_state.swapChainRebuild = false;

            fprintf(stderr, "[vulkan] Swapchain recreated: %ux%u\n", wd->Width, wd->Height);
        }

        // ------------------------------------------------------------------
        // Minimized
        // ------------------------------------------------------------------

        if (glfwGetWindowAttrib(m_state.window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // ------------------------------------------------------------------
        // Start ImGui frame
        // ------------------------------------------------------------------

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        if (user_interface)
            user_interface->Render();

        // ------------------------------------------------------------------
        // Render ImGui
        // ------------------------------------------------------------------

        ImGui::Render();

        ImDrawData* main_draw_data = ImGui::GetDrawData();

        const bool main_is_minimized =
            main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f;

        if (!main_is_minimized)
        {
            FrameRender(m_state, wd, main_draw_data);
        }

        // ------------------------------------------------------------------
        // Multi-viewport
        // ------------------------------------------------------------------

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();

            ImGui::RenderPlatformWindowsDefault();
        }

        // ------------------------------------------------------------------
        // Present
        // ------------------------------------------------------------------

        if (!main_is_minimized)
        {
            FramePresent(m_state, wd);
        }
    }
}

void VulkanRenderer::Cleanup()
{
    LOG_SCOPE("::Cleanup()");

    if (m_cleaned_up)
        return;

    m_cleaned_up = true;

    // ----------------------------------------------------------------------
    // Wait for GPU
    // ----------------------------------------------------------------------

    if (m_state.device != VK_NULL_HANDLE)
    {
        VkResult err = vkDeviceWaitIdle(m_state.device);

        if (err != VK_SUCCESS)
        {
            fprintf(
                stderr, "[vulkan] vkDeviceWaitIdle during cleanup failed: %s\n", VkResultToString(err));
        }
    }

    // ----------------------------------------------------------------------
    // ImGui
    // ----------------------------------------------------------------------

    if (m_state.device != VK_NULL_HANDLE)
    {
        ImGui_ImplVulkan_Shutdown();
    }

    ImGui_ImplGlfw_Shutdown();

    if (ImGui::GetCurrentContext() != nullptr)
    {
        ImGui::DestroyContext();
    }

    // ----------------------------------------------------------------------
    // Vulkan
    // ----------------------------------------------------------------------

    if (m_state.device != VK_NULL_HANDLE)
    {
        CleanupVulkanWindow(m_state, &m_state.mainWindowData);
    }

    CleanupVulkan(m_state);

    // ----------------------------------------------------------------------
    // GLFW
    // ----------------------------------------------------------------------

    if (m_state.window != nullptr)
    {
        glfwDestroyWindow(m_state.window);

        m_state.window = nullptr;
    }

    glfwTerminate();
}

} // namespace Graphite::Application::Renderer::Backends::Vulkan
