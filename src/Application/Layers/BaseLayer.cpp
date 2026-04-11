/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BaseLayer.cpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Implementation of @see BaseLayer.hpp.
///

#include <filesystem>

#include "BaseLayer.hpp"

#include "Graphite/Logger.hpp"
#include "imgui.h"

DEFINE_LOG_SCOPE(Fluxion::Application::Layers::BaseLayer);
USE_LOG_SCOPE(Fluxion::Application::Layers::BaseLayer);

namespace Fluxion::Application::Layers {

// Default ImGui layout for first launch
static const char* DEFAULT_IMGUI_LAYOUT = R"(
[Window][WindowOverViewport_11111111]
Pos=0,22
Size=950,728
Collapsed=0

[Window][Debug##Default]
Pos=60,60
Size=400,400
Collapsed=0

[Window][ Logs]
Pos=0,22
Size=950,482
Collapsed=0
DockId=0x00000001,0

[Window][ Dev]
Pos=0,22
Size=950,482
Collapsed=0
DockId=0x00000001,1

[Window][ Filters]
Pos=0,506
Size=950,244
Collapsed=0
DockId=0x00000002,0

[Table][0xB237F4D5,2]
RefScale=16
Column 0  Width=232
Column 1  Width=291

[Table][0x4DE70969,4]
RefScale=16
Column 0  Width=139
Column 1  Width=59
Column 2  Width=37
Column 3  Width=379

[Docking][Data]
DockSpace   ID=0x08BD597D Window=0x1BBC0F80 Pos=0,22 Size=950,728 Split=Y
  DockNode  ID=0x00000001 Parent=0x08BD597D SizeRef=950,482 CentralNode=1 Selected=0xF82A8BBB
  DockNode  ID=0x00000002 Parent=0x08BD597D SizeRef=950,244 Selected=0x28C6CA5A


)";

std::string_view BaseLayer::GetLayerName() noexcept
{
    return "BaseLayer";
}

std::string_view BaseLayer::GetName() const noexcept
{
    return BaseLayer::GetLayerName();
}

BaseLayer::BaseLayer(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Layers::ZIndex const z_index)
    : TLayer{std::move(application), z_index}
{
    LOG_SCOPE("::BaseLayer()");
}

void BaseLayer::OnAdd()
{
    LOG_SCOPE("::OnAdd()");
    if (ImGui::GetCurrentContext() == nullptr)
    {
        ImGui::CreateContext();
    }

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // --- Resolve Custom Path ---
    static std::string persistent_path{}; // Must stay alive for ImGui
    const char* home = std::getenv("HOME");

#ifdef _WIN32
    if (!home)
    {
        home = std::getenv("USERPROFILE");
    }
#endif

    if (home)
    {
        std::filesystem::path config_dir = std::filesystem::path(home) / ".fluxion";
        std::filesystem::create_directories(config_dir); // Ensure folder exists
        persistent_path = (config_dir / "fluxion.ini").string();
        io.IniFilename = persistent_path.c_str();
    }
    else
    {
        LOG_WARN("Could not find home path...");
    }

    // --- Set Default Layout ---
    if (io.IniFilename && !std::filesystem::exists(io.IniFilename))
    {
        LOG_INFO("imgui.ini not found, loading DEFAULT_IMGUI_LAYOUT");
        ImGui::LoadIniSettingsFromMemory(DEFAULT_IMGUI_LAYOUT);
    }
}

void BaseLayer::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
}

void BaseLayer::OnRender()
{
    LOG_SCOPE("::OnRender()");
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);
}

void BaseLayer::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

} // namespace Fluxion::Application::Layers
