/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DummyLogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.10
/// @brief Dummy implementation of a LogsPlugin
///

#include <vector>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"
#include "Fluxion/API/LogsPlugin/PluginBridge.hpp"
#include "Graphite/Common/Plugin/GraphiteExport.hpp"

namespace Fluxion::Plugins::Logs::DummyLogsPlugin {

class DummyLogsPlugin : public Fluxion::API::LogsPlugin::IFluxionLogsPlugin
{
public:
    DummyLogsPlugin();

public:
    std::string_view GetDisplayName() const override final;

    void OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& data) override final;
    void OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& data) override final;

    void RenderMenu() override final;

    void ImportLogs(std::filesystem::path const& path) override final;

    std::optional<std::size_t> GetNextLog(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index = 0) override final;
    std::optional<std::size_t> GetPrevLog(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index = 0) override final;

    void ApplyFilters(
        std::vector<Fluxion::API::LogsPlugin::Data::Filter> filters,
        std::vector<Fluxion::API::LogsPlugin::Data::Filter> highlight_only) override final;
    void DisableFilters() override final;

    std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> GetTableHeader() const override final;

    std::size_t GetTotalLogs() const override final;

    void GetLogs(
        std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
        Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter out_logs) override final;

    std::size_t GetLogsOperationTarget() const override final;
    std::size_t GetLogsOperationProgress() const override final;
    Fluxion::API::LogsPlugin::Data::ELogsOperationUnit GetLogsOperationUnit() const override final;

private:
    std::vector<std::vector<std::string>> m_logs;
    std::vector<Fluxion::API::LogsPlugin::Data::LogRow> m_filtered_logs;
};

} // namespace Fluxion::Plugins::Logs::DummyLogsPlugin

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin();
typedef Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*CreateFluxionLogsPluginFactory)();
