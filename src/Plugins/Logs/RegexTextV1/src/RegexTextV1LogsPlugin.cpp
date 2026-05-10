/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RegexTextV1LogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 0.8
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#include <string>

#include "Fluxion/Plugins/Logs/RegexTextV1/RegexTextV1LogsPlugin.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);
// USE_LOG_SCOPE(Fluxion::Plugins::Logs::RegexTextV1);

namespace Fluxion::Plugins::Logs::RegexTextV1 {

using RegexTags = std::vector<std::shared_ptr<Data::RegexTag>>;

std::string_view RegexTextV1LogsPlugin::GetDisplayName() const
{
    return "RegexTextV1";
}

std::filesystem::path RegexTextV1LogsPlugin::MakeConvertedLogsPath(
    std::filesystem::path const& raw_logs_path) const
{
    auto const output_path{m_home_path / (raw_logs_path.filename().string() + ".converted.csv")};
    return output_path;
}

std::filesystem::path RegexTextV1LogsPlugin::MakeFilteredLogsPath(
    std::filesystem::path const& raw_logs_path) const
{
    auto const output_path{m_home_path / (raw_logs_path.filename().string() + ".filtered.csv")};
    return output_path;
}

Graphite::Settings::PersistentSettings RegexTextV1LogsPlugin::GetConfig() const
{
    return Graphite::Settings::PersistentSettings{m_home_path, "config"};
}

} // namespace Fluxion::Plugins::Logs::RegexTextV1

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin()
{
    return new Fluxion::Plugins::Logs::RegexTextV1::RegexTextV1LogsPlugin();
}
