/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RegexTextV1LogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#include <memory>
#include <vector>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"
#include "Graphite/Common/DataStructures/TDoubleBuffer.hpp"
#include "Graphite/Common/Plugin/GraphiteExport.hpp"
#include "Graphite/Settings/PersistentSettings.hpp"

#include "Data.hpp"

namespace Fluxion::Plugins::Logs::RegexTextV1 {

class RegexTextV1LogsPlugin : public Fluxion::API::LogsPlugin::IFluxionLogsPlugin
{
public:
    std::string_view GetDisplayName() const override;

    void OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& data) override;
    void OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& data) override;

    void RenderMenu() override;

    void ImportLogs(std::filesystem::path const& path) override;

    std::optional<std::size_t> GetNextLog(Graphite::Common::Utility::UniqueID const& filter_id) override;
    std::optional<std::size_t> GetPrevLog(Graphite::Common::Utility::UniqueID const& filter_id) override;

    void ApplyFilters(
        std::vector<Fluxion::API::LogsPlugin::Data::Filter> filters,
        std::vector<Fluxion::API::LogsPlugin::Data::Filter> highlight_only) override;
    void DisableFilters() override;

    std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> GetTableHeader() const override;

    std::size_t GetTotalLogs() const override;

    void GetLogs(
        std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
        Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter out_logs) const override;

private:
    std::filesystem::path MakeConvertedLogsPath(std::filesystem::path const& raw_logs_path) const;
    std::filesystem::path MakeFilteredLogsPath(std::filesystem::path const& raw_logs_path) const;
    Graphite::Settings::PersistentSettings GetConfig() const;

    void SaveRegexTags(std::vector<std::shared_ptr<Data::RegexTag>> const& tags) const;
    std::vector<std::shared_ptr<Data::RegexTag>> LoadRegexTags() const;
    void UpdateImportedLogsHeader(std::vector<std::shared_ptr<Data::RegexTag>> const& tags);

private:
    Graphite::Common::DataStructures::TCopyDoubleBuffer<std::vector<std::shared_ptr<Data::RegexTag>>>
        m_regex_tags{};

    std::filesystem::path m_home_path{};
    std::optional<std::filesystem::path> m_last_imported_logs_path{};
    std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> m_imported_logs_header{};
};

} // namespace Fluxion::Plugins::Logs::RegexTextV1

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin();
typedef Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*CreateFluxionLogsPluginFactory)();
