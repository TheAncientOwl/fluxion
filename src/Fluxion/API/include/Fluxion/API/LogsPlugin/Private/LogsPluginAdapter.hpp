/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FluxionLogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Host implementation of @see IFluxionLogsPlugin.hpp
///

#pragma once

#include <vector>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"
#include "Fluxion/API/LogsPlugin/Private/LogsPluginAPI.hpp"

namespace Fluxion::API::LogsPlugin::Private {

class LogsPluginAdapter final : public IFluxionLogsPlugin
{
public:
    LogsPluginAdapter() = default;
    explicit LogsPluginAdapter(LogsPluginAPI api);

    ~LogsPluginAdapter() override;

    LogsPluginAdapter(LogsPluginAdapter const&) = delete;
    LogsPluginAdapter& operator=(LogsPluginAdapter const&) = delete;

    LogsPluginAdapter(LogsPluginAdapter&& other) noexcept;
    LogsPluginAdapter& operator=(LogsPluginAdapter&& other) noexcept;

    std::string_view GetDisplayName() const override;
    std::string_view GetDirectoryName() const override;

    void OnEnable(OnEnableData const& data) override;
    void OnDisable(OnDisableData const& data) override;

    void RenderMenu() override;

    void ImportLogs(std::filesystem::path const& path) override;

    std::optional<std::size_t> GetNextLog(
        UniqueID const& filter_id,
        std::size_t const current_index = 0) override;

    std::optional<std::size_t> GetPrevLog(
        UniqueID const& filter_id,
        std::size_t const current_index = 0) override;

    void ApplyFilters(
        std::span<Filter const> const filters,
        std::span<Filter const> const highlight_only) override;

    void DisableFilters() override;

    std::span<ColumnDetails const> GetTableHeader() const override;

    std::size_t GetTotalLogs() const override;

    void GetLogs(
        std::span<ABI::Safe::Range const> const ranges,
        Fluxion::API::LogsPlugin::WriteLogRowDataFn write_data,
        Fluxion::API::LogsPlugin::WriteLogRowMetadataFn write_metadata,
        void* user_data) override;

    std::size_t GetLogsOperationTarget() const override;

    ABI::Safe::ELogsOperationUnit GetLogsOperationUnit() const override;

    std::size_t GetLogsOperationProgress() const override;

private:
    LogsPluginAPI m_api{};
    mutable std::vector<ColumnDetails> m_table_header{};
};

} // namespace Fluxion::API::LogsPlugin::Private
