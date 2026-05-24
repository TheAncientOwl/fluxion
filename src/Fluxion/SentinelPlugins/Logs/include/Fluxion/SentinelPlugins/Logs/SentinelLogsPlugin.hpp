/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SentinelLogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Do nothing...
///

#include <memory>
#include <vector>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"
#include "Fluxion/API/LogsPlugin/PluginBridge.hpp"

namespace Fluxion::SentinelPlugins::Logs {

class SentinelLogsPlugin : public Fluxion::API::LogsPlugin::IFluxionLogsPlugin
{
public:
    SentinelLogsPlugin();

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
    std::vector<std::vector<std::string>> m_logs;
    std::vector<Fluxion::API::LogsPlugin::Data::LogRow> m_filtered_logs;

    std::unordered_map<Graphite::Common::Utility::UniqueID, std::optional<std::size_t>>
        m_filter_to_search_log_index{};
};

std::unique_ptr<Fluxion::API::LogsPlugin::IFluxionLogsPlugin> Create();

} // namespace Fluxion::SentinelPlugins::Logs
