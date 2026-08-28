/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ThemeSerializer.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief ImGui theme serializer helpers
///

#include <fstream>
#include <imgui.h>
#include <nlohmann/json.hpp>

#include "Graphite/Common/Utility/ThemeSerializer.hpp"

namespace Graphite::Common::Utility::Theme {

void SaveThemeToJson(std::filesystem::path const& path)
{
    ImGuiStyle& style = ImGui::GetStyle();
    nlohmann::json j;

    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        const char* name = ImGui::GetStyleColorName(i);
        const ImVec4& c = style.Colors[i];
        j["colors"][name] = {c.x, c.y, c.z, c.w};
    }

    std::ofstream file(path);
    if (file.is_open())
    {
        file << j.dump(4);
    }
}

bool LoadThemeFromJson(std::filesystem::path const& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    nlohmann::json j;
    file >> j;
    if (!j.contains("colors"))
        return false;

    ImGuiStyle& style = ImGui::GetStyle();

    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        const char* name = ImGui::GetStyleColorName(i);
        if (j["colors"].contains(name))
        {
            auto& col = j["colors"][name];
            style.Colors[i] = ImVec4(
                col[0].get<float>(), col[1].get<float>(), col[2].get<float>(), col[3].get<float>());
        }
    }
    return true;
}

} // namespace Graphite::Common::Utility::Theme
