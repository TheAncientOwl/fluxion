/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file BaseView.cpp
/// @author Alexandru Delegeanu
/// @version 0.8
/// @brief Implementation of @see BaseView.hpp.
///

#include <filesystem>

#include "BaseView.hpp"

#include "Graphite/Logger.hpp"
#include "imgui.h"

DEFINE_LOG_SCOPE(Fluxion::Application::Views::BaseView);
USE_LOG_SCOPE(Fluxion::Application::Views::BaseView);

namespace Fluxion::Application::Views {

// Default ImGui layout for first launch
static const char* DEFAULT_IMGUI_LAYOUT = R"(
[Window][WindowOverViewport_11111111]
Pos=0,22
Size=950,728
Collapsed=0

[Window][Debug##Default]
Pos=68,104
Size=400,400
Collapsed=0

[Window][ Logs]
Pos=0,22
Size=950,364
Collapsed=0
DockId=0x00000001,0

[Window][ Dev]
Pos=0,22
Size=950,364
Collapsed=0
DockId=0x00000001,2

[Window][ Filters]
Pos=0,388
Size=950,362
Collapsed=0
DockId=0x00000002,0

[Window][Select Log File to Import]
Pos=132,71
Size=700,500
Collapsed=0

[Window][Settings]
Pos=0,22
Size=950,464
Collapsed=0
DockId=0x00000001,3

[Window][ Settings]
Pos=0,22
Size=950,364
Collapsed=0
DockId=0x00000001,1

[Window][ Debug]
Pos=211,160
Size=377,174
Collapsed=0

[Window][ Logs Progress]
Pos=144,138
Size=600,100
Collapsed=0

[Window][ Logs Progress]
Pos=146,113
Size=443,183
Collapsed=0

[Window][ Logs Progress]
Pos=121,169
Size=855,195
Collapsed=0

[Window][ Logs Progress]
Pos=102,97
Size=352,143
Collapsed=0

[Window][ Select Log File to Import]
Pos=60,60
Size=700,500
Collapsed=0

[Window][ Select Logs File to Import]
Pos=165,151
Size=700,500
Collapsed=0

[Window][Import Settings]
Pos=115,98
Size=168,60
Collapsed=0

[Window][ About]
Pos=0,22
Size=950,364
Collapsed=0
DockId=0x00000001,3

[Window][ About]
Pos=0,22
Size=950,364
Collapsed=0
DockId=0x00000001,3

[Table][0xB237F4D5,2]
RefScale=16
Column 0  Width=249
Column 1  Width=575

[Table][0x4DE70969,4]
RefScale=16
Column 0  Width=66
Column 1  Width=59
Column 2  Width=59
Column 3  Width=652

[Table][0x487683E2,4]
RefScale=16
Column 0  Width=66
Column 1  Width=59
Column 2  Width=37
Column 3  Width=5811

[Table][0x210F1456,5]
RefScale=16
Column 0  Width=51
Column 1  Width=66
Column 2  Width=59
Column 3  Width=37
Column 4  Width=5811

[Table][0x1BE8D7FC,5]
RefScale=16
Column 0  Width=51
Column 1  Width=66
Column 2  Width=59
Column 3  Width=37
Column 4  Width=5571

[Docking][Data]
DockSpace   ID=0x08BD597D Window=0x1BBC0F80 Pos=0,22 Size=950,728 Split=Y Selected=0xF82A8BBB
  DockNode  ID=0x00000001 Parent=0x08BD597D SizeRef=950,364 CentralNode=1 Selected=0xF82A8BBB
  DockNode  ID=0x00000002 Parent=0x08BD597D SizeRef=950,362 Selected=0x28C6CA5A

)";

std::string_view BaseView::GetViewName() noexcept
{
    return "BaseView";
}

std::string_view BaseView::GetName() const noexcept
{
    return BaseView::GetViewName();
}

BaseView::BaseView(
    FluxionApplication::FluxionApplication::Ptr application,
    Graphite::Application::Views::RenderPriority const render_priority)
    : TView{std::move(application), render_priority}
{
    LOG_SCOPE("::BaseView()");
}

void BaseView::OnAdd()
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
    persistent_path =
        (m_application->As<FluxionApplication>()->GetHomePath() / "fluxion.ini").string();
    io.IniFilename = persistent_path.c_str();

    // --- Set Default Layout ---
    if (io.IniFilename && !std::filesystem::exists(io.IniFilename))
    {
        LOG_INFO("imgui.ini not found, loading DEFAULT_IMGUI_LAYOUT");
        ImGui::LoadIniSettingsFromMemory(DEFAULT_IMGUI_LAYOUT);
    }
}

void BaseView::OnIterate()
{
    LOG_SCOPE("::OnIterate()");
}

void BaseView::OnRender()
{
    LOG_SCOPE("::OnRender()");
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);
}

void BaseView::OnRemove()
{
    LOG_SCOPE("::OnRemove()");
}

} // namespace Fluxion::Application::Views
