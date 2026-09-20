/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SentinelLogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Do nothing...
///

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"
#include "Fluxion/API/LogsPlugin/Private/LogsPluginAPI.hpp"

namespace Fluxion::SentinelLogsPlugin {

class SentinelLogsPlugin final : public Fluxion::API::LogsPlugin::IFluxionLogsPlugin
{
public:
    SentinelLogsPlugin();

public:
    std::string_view GetDisplayName() const override;
    std::string_view GetDirectoryName() const override;

    void OnEnable(Fluxion::API::LogsPlugin::OnEnableData const& data) override;
    void OnDisable(Fluxion::API::LogsPlugin::OnDisableData const& data) override;

    void RenderMenu() override;

    void ImportLogs(std::filesystem::path const& path) override;

    std::optional<std::size_t> GetNextLog(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index = 0) override;
    std::optional<std::size_t> GetPrevLog(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index = 0) override;

    void ApplyFilters(
        std::span<Fluxion::API::LogsPlugin::Filter const> const filters,
        std::span<Fluxion::API::LogsPlugin::Filter const> const highlight_only) override;
    void DisableFilters() override;

    std::span<Fluxion::API::LogsPlugin::ColumnDetails const> GetTableHeader() const override;

    std::size_t GetTotalLogs() const override;

    void GetLogs(
        std::span<Fluxion::API::LogsPlugin::Range const> const ranges,
        Fluxion::API::LogsPlugin::WriteLogRowDataFn write_data,
        Fluxion::API::LogsPlugin::WriteLogRowMetadataFn write_metadata,
        void* user_data) override;

    std::size_t GetLogsOperationTarget() const override;
    std::size_t GetLogsOperationProgress() const override;
    Fluxion::API::LogsPlugin::ELogsOperationUnit GetLogsOperationUnit() const override;

private:
    std::vector<std::vector<std::string>> m_logs;
    std::vector<Fluxion::API::LogsPlugin::Private::ABI::Unsafe::LogRow> m_filtered_logs;

    std::unordered_map<Graphite::Common::Utility::UniqueID, std::optional<std::size_t>>
        m_filter_to_search_log_index{};
};

Fluxion::API::LogsPlugin::Private::LogsPluginAPI Create();

} // namespace Fluxion::SentinelLogsPlugin
