
/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Utilityy.hpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Utilities
///

#pragma once

#include <memory>
#include <vector>

#include "Fluxion/API/LogsPlugin/PluginBridge.hpp"
#include "Fluxion/Plugins/Logs/Text/RegexTags/V7/Data.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite::Utility {

[[nodiscard]] std::vector<std::string> MakeFieldsIDs(
    std::vector<std::shared_ptr<Data::RegexTag>> const& tags);

[[nodiscard]] std::vector<std::string> MakeFieldsIDs(
    std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> const& columns);

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::SQLite::Utility
