/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DummyLogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.11
/// @brief Dummy implementation of a LogsPlugin
///

#include <vector>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"

namespace Fluxion::LogsPlugin::DummyLogsPlugin {

class DummyLogsPlugin final : public Fluxion::API::LogsPlugin::IFluxionLogsPlugin
{
public:
    DummyLogsPlugin();

public:
    std::string_view GetDisplayName() const override;
    std::string_view GetDirectoryName() const override;

    void OnEnable(Fluxion::API::LogsPlugin::OnEnableData const& data) override;
    void OnDisable(Fluxion::API::LogsPlugin::OnDisableData const& data) override;

    void RenderMenu() override;

    void ImportLogs(std::filesystem::path const& path) override;

    std::optional<std::size_t> GetNextLog(
        Fluxion::API::LogsPlugin::UniqueID const& filter_id,
        std::size_t const current_index = 0) override;
    std::optional<std::size_t> GetPrevLog(
        Fluxion::API::LogsPlugin::UniqueID const& filter_id,
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
    std::vector<Fluxion::API::LogsPlugin::OwningLogRow> m_filtered_logs;
};

} // namespace Fluxion::LogsPlugin::DummyLogsPlugin
