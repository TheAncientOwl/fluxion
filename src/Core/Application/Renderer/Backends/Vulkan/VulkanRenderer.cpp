/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file VulkanRenderer.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Implementation of @see VulkanRenderer.hpp.
///

#include "VulkanRenderer.hpp"
#include "Core/Logger/Logger.hpp"

#include <exception>

#include <stdio.h> // printf, fprintf
#include <stdlib.h> // abort
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Volk headers
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of
// testing and compatibility with old VS compilers. To link with VS2010-era libraries, VS2015+
// requires linking with legacy_stdio_definitions.lib, which we do using this pragma. Your own
// project should not be affected, as you are likely to link with a newer binary of GLFW that is
// adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// #define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

static void glfw_error_callback(int error, const char* description)
{
    // LOG_ERROR("GLFW Error {}: {}", error, description);
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
static void check_vk_result(VkResult err)
{
    if (err == VK_SUCCESS)
        return;
    // LOG_ERROR("[vulkan] Error: VkResult = {}", err);
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

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
    (void)pLayerPrefix; // Unused arguments
    LOG_DEBUG("[vulkan] Debug report from ObjectType: {} Message: {}", objectType, pMessage);
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // APP_USE_VULKAN_DEBUG_REPORT

static bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension)
{
    for (const VkExtensionProperties& p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    return false;
}

static void SetupVulkan(
    ::Graphite::Core::Renderer::Backends::Vulkan::VulkanRenderer::State& state,
    ImVector<const char*> instance_extensions)
{
    VkResult err;
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
    volkInitialize();
#endif

    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        // Enumerate available extensions
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.Data);
        check_vk_result(err);

        // Enable required extensions
        if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
            instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
        {
            instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
#endif

        // Enabling validation layers
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers;
        instance_extensions.push_back("VK_EXT_debug_report");
#endif

        // Create Vulkan Instance
        create_info.enabledExtensionCount = (uint32_t)instance_extensions.Size;
        create_info.ppEnabledExtensionNames = instance_extensions.Data;
        err = vkCreateInstance(&create_info, state.allocator, &state.instance);
        check_vk_result(err);
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
        volkLoadInstance(state.instance);
#endif

        // Setup the debug report callback
#ifdef APP_USE_VULKAN_DEBUG_REPORT
        auto f_vkCreateDebugReportCallbackEXT =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
                state.instance, "vkCreateDebugReportCallbackEXT");
        IM_ASSERT(f_vkCreateDebugReportCallbackEXT != nullptr);
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = nullptr;
        err = f_vkCreateDebugReportCallbackEXT(
            state.instance, &debug_report_ci, state.allocator, &state.debugReport);
        check_vk_result(err);
#endif
    }

    // Select Physical Device (GPU)
    state.physicalDevice = ImGui_ImplVulkanH_SelectPhysicalDevice(state.instance);
    IM_ASSERT(state.physicalDevice != VK_NULL_HANDLE);

    // Select graphics queue family
    state.queueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(state.physicalDevice);
    IM_ASSERT(state.queueFamily != (uint32_t)-1);

    // Create Logical Device (with 1 queue)
    {
        ImVector<const char*> device_extensions;
        device_extensions.push_back("VK_KHR_swapchain");

        // Enumerate physical device extension
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateDeviceExtensionProperties(state.physicalDevice, nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        vkEnumerateDeviceExtensionProperties(
            state.physicalDevice, nullptr, &properties_count, properties.Data);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
            device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

        const float queue_priority[] = {1.0f};
        VkDeviceQueueCreateInfo queue_info[1] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = state.queueFamily;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority;
        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info;
        create_info.enabledExtensionCount = (uint32_t)device_extensions.Size;
        create_info.ppEnabledExtensionNames = device_extensions.Data;
        err = vkCreateDevice(state.physicalDevice, &create_info, state.allocator, &state.device);
        check_vk_result(err);
        vkGetDeviceQueue(state.device, state.queueFamily, 0, &state.queue);
    }

    // Create Descriptor Pool
    // If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
    {
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
             IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE},
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 0;
        for (VkDescriptorPoolSize& pool_size : pool_sizes)
            pool_info.maxSets += pool_size.descriptorCount;
        pool_info.poolSizeCount = (uint32_t)IM_COUNTOF(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(state.device, &pool_info, state.allocator, &state.descriptorPool);
        check_vk_result(err);
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void SetupVulkanWindow(
    ::Graphite::Core::Renderer::Backends::Vulkan::VulkanRenderer::State& state,
    ImGui_ImplVulkanH_Window* wd,
    VkSurfaceKHR surface,
    int width,
    int height)
{
    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(state.physicalDevice, state.queueFamily, surface, &res);
    if (res != VK_TRUE)
    {
        // LOG_CRITICAL("Error no WSI support on physical device 0");
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = {
        VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->Surface = surface;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
        state.physicalDevice,
        wd->Surface,
        requestSurfaceImageFormat,
        (size_t)IM_COUNTOF(requestSurfaceImageFormat),
        requestSurfaceColorSpace);

    // Select Present Mode
#ifdef APP_USE_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = {
        VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR};
#else
    VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
        state.physicalDevice, wd->Surface, &present_modes[0], IM_COUNTOF(present_modes));
    // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(state.minImageCount >= 2);
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
}

static void CleanupVulkan(::Graphite::Core::Renderer::Backends::Vulkan::VulkanRenderer::State& state)
{
    vkDestroyDescriptorPool(state.device, state.descriptorPool, state.allocator);

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto f_vkDestroyDebugReportCallbackEXT =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
            state.instance, "vkDestroyDebugReportCallbackEXT");
    f_vkDestroyDebugReportCallbackEXT(state.instance, state.debugReport, state.allocator);
#endif // APP_USE_VULKAN_DEBUG_REPORT

    vkDestroyDevice(state.device, state.allocator);
    vkDestroyInstance(state.instance, state.allocator);
}

static void CleanupVulkanWindow(
    ::Graphite::Core::Renderer::Backends::Vulkan::VulkanRenderer::State& state,
    ImGui_ImplVulkanH_Window* wd)
{
    ImGui_ImplVulkanH_DestroyWindow(state.instance, state.device, wd, state.allocator);
    vkDestroySurfaceKHR(state.instance, wd->Surface, state.allocator);
}

static void FrameRender(
    ::Graphite::Core::Renderer::Backends::Vulkan::VulkanRenderer::State& state,
    ImGui_ImplVulkanH_Window* wd,
    ImDrawData* draw_data)
{
    VkSemaphore image_acquired_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkResult err = vkAcquireNextImageKHR(
        state.device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        state.swapChainRebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    if (err != VK_SUBOPTIMAL_KHR)
        check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(
            state.device,
            1,
            &fd->Fence,
            VK_TRUE,
            UINT64_MAX); // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(state.device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(state.device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(state.queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

static void FramePresent(
    ::Graphite::Core::Renderer::Backends::Vulkan::VulkanRenderer::State& state,
    ImGui_ImplVulkanH_Window* wd)
{
    if (state.swapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(state.queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        state.swapChainRebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    if (err != VK_SUBOPTIMAL_KHR)
        check_vk_result(err);
    wd->SemaphoreIndex =
        (wd->SemaphoreIndex + 1) % wd->SemaphoreCount; // Now we can use the next set of semaphores
}

namespace Graphite::Core::Renderer::Backends::Vulkan {

VulkanRenderer::VulkanRenderer()
{
    LOG_SCOPE("");
}

VulkanRenderer::~VulkanRenderer()
{
    LOG_SCOPE("");
    Cleanup();
}

void VulkanRenderer::Init(Graphite::Core::Application::WindowConfiguration const& window_configuration)
{
    LOG_SCOPE("");

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        std::cerr << "::Graphite::Core::Application::Renderer::Backends::Vulkan::Vulkan(): "
                     "[Critical] could not init glfw";
        std::terminate();
    }

    // Create window with Vulkan context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    float main_scale =
        ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    m_state.window = glfwCreateWindow(
        (int)(window_configuration.width * main_scale),
        (int)(window_configuration.height * main_scale),
        window_configuration.title.c_str(),
        nullptr,
        nullptr);
    if (!glfwVulkanSupported())
    {
        printf("GLFW: Vulkan Not Supported\n");
        std::terminate();
    }

    ImVector<const char*> extensions;
    uint32_t extensions_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
    for (uint32_t i = 0; i < extensions_count; i++)
        extensions.push_back(glfw_extensions[i]);
    SetupVulkan(m_state, extensions);

    // Create Window Surface
    VkSurfaceKHR surface;
    VkResult err =
        glfwCreateWindowSurface(m_state.instance, m_state.window, m_state.allocator, &surface);
    check_vk_result(err);

    // Create Framebuffers
    int w, h;
    glfwGetFramebufferSize(m_state.window, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &m_state.mainWindowData;
    SetupVulkanWindow(m_state, wd, surface, w, h);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(
        main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style
                     // scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi =
        main_scale; // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true
                    // automatically overrides this for every window depending on the current monitor)
    io.ConfigDpiScaleFonts =
        true; // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor
              // DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports =
        true; // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(m_state.window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    // init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of
    // VkApplicationInfo::apiVersion, otherwise will default to header version.
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

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either
    // AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi)
    //   reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those
    // errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher
    // quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to
    // write a double backslash \\ !
    // style.FontSizeBase = 20.0f;
    // io.Fonts->AddFontDefaultVector();
    // io.Fonts->AddFontDefaultBitmap();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    // IM_ASSERT(font != nullptr);
}

void VulkanRenderer::Render(std::shared_ptr<IRenderable> user_interface)
{
    LOG_SCOPE("Main Render Loop");

    ImGui_ImplVulkanH_Window* wd = &m_state.mainWindowData;
    while (!glfwWindowShouldClose(m_state.window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application,
        // or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
        // application, or clear/overwrite your copy of the keyboard data. Generally you may always
        // pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Resize swap chain?
        int fb_width, fb_height;
        glfwGetFramebufferSize(m_state.window, &fb_width, &fb_height);
        if (fb_width > 0 && fb_height > 0 &&
            (m_state.swapChainRebuild || m_state.mainWindowData.Width != fb_width ||
             m_state.mainWindowData.Height != fb_height))
        {
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
            m_state.mainWindowData.FrameIndex = 0;
            m_state.swapChainRebuild = false;
        }
        if (glfwGetWindowAttrib(m_state.window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        user_interface->Render();

        // Rendering
        ImGui::Render();
        ImDrawData* main_draw_data = ImGui::GetDrawData();
        const bool main_is_minimized =
            (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
        if (!main_is_minimized)
            FrameRender(m_state, wd, main_draw_data);

        // Update and Render additional Platform Windows
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // Present Main Platform Window
        if (!main_is_minimized)
            FramePresent(m_state, wd);
    }
}

void VulkanRenderer::Cleanup()
{
    LOG_SCOPE("");

    if (!m_cleaned_up)
    {
        m_cleaned_up = true;

        VkResult err = vkDeviceWaitIdle(m_state.device);
        check_vk_result(err);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        CleanupVulkanWindow(m_state, &m_state.mainWindowData);
        CleanupVulkan(m_state);

        glfwDestroyWindow(m_state.window);
        glfwTerminate();
    }
}

} // namespace Graphite::Core::Renderer::Backends::Vulkan
