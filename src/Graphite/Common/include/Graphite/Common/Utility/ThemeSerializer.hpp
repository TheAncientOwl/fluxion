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

#pragma once

#include <filesystem>

namespace Graphite::Common::Utility::Theme {

void SaveThemeToJson(std::filesystem::path const& path);

bool LoadThemeFromJson(std::filesystem::path const& path);

} // namespace Graphite::Common::Utility::Theme
