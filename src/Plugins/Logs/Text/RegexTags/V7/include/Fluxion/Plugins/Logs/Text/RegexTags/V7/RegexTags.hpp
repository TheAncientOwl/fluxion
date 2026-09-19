/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RegexTags.hpp
/// @author Alexandru Delegeanu
/// @version 7.1
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#pragma once

#include <memory>
#include <vector>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"

#include "Graphite/Common/DataStructures/TDoubleBuffer.hpp"
#include "Graphite/Common/Plugin/GraphiteExport.hpp"
#include "Graphite/Settings/PersistentSettings.hpp"
#include "Scribe.hpp"

#include "Data.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7 {

class GRAPHITE_EXPORT RegexTags final : public Fluxion::API::LogsPlugin::IFluxionLogsPlugin
{
public:
    RegexTags();
    ~RegexTags() override;

    std::string_view GetDisplayName() const override final;
    std::string_view GetDirectoryName() const override final;

    void OnEnable(Fluxion::API::LogsPlugin::OnEnableData const& data) override;
    void OnDisable(Fluxion::API::LogsPlugin::OnDisableData const& data) override;

    void RenderMenu() override final;

    void ImportLogs(std::filesystem::path const& path) override final;

    std::optional<std::size_t> GetNextLog(
        Fluxion::API::LogsPlugin::UniqueID const& filter_id,
        std::size_t const current_index) override final;
    std::optional<std::size_t> GetPrevLog(
        Fluxion::API::LogsPlugin::UniqueID const& filter_id,
        std::size_t const current_index) override final;

    void ApplyFilters(
        std::span<Fluxion::API::LogsPlugin::Filter const> const filters,
        std::span<Fluxion::API::LogsPlugin::Filter const> const highlight_only) override;
    void DisableFilters() override final;

    std::span<Fluxion::API::LogsPlugin::ColumnDetails const> GetTableHeader() const override;

    std::size_t GetTotalLogs() const override final;

    void GetLogs(
        std::span<Fluxion::API::LogsPlugin::Range const> const ranges,
        Fluxion::API::LogsPlugin::WriteLogRowDataFn write_data,
        Fluxion::API::LogsPlugin::WriteLogRowMetadataFn write_metadata,
        void* user_data) override;

    std::size_t GetLogsOperationTarget() const override final;
    std::size_t GetLogsOperationProgress() const override final;
    Fluxion::API::LogsPlugin::ELogsOperationUnit GetLogsOperationUnit() const override;

private:
    std::filesystem::path MakeDatabasePath(std::filesystem::path const& raw_logs_path) const;
    Graphite::Settings::PersistentSettings GetConfig() const;

    void SaveRegexTags(std::vector<std::shared_ptr<Data::RegexTag>> const& tags) const;
    std::vector<std::shared_ptr<Data::RegexTag>> LoadRegexTags() const;
    void UpdateImportedLogsHeader(std::vector<std::shared_ptr<Data::RegexTag>> const& tags);

    void LoadSettings();
    void SaveSettings() const;

private:
    Graphite::Common::DataStructures::TCopyDoubleBuffer<std::vector<std::shared_ptr<Data::RegexTag>>>
        m_regex_tags{};

    std::filesystem::path m_home_path{};
    std::optional<std::filesystem::path> m_last_imported_logs_path{};
    std::vector<Fluxion::API::LogsPlugin::ColumnDetails> m_imported_logs_header{};
    std::atomic<std::size_t> m_logs_operation_progress{0};
    std::size_t m_logs_operation_target{0};
    Fluxion::API::LogsPlugin::ELogsOperationUnit m_logs_operation_unit{
        Fluxion::API::LogsPlugin::ELogsOperationUnit::Logs};

    Scrolls::Scribe m_scrolls{};
    std::vector<Data::FilteredLog> m_filtered_logs{};
    std::size_t m_total_logs_imported{0};

    Data::Settings m_settings{};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin();
typedef Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*CreateFluxionLogsPluginFactory)();
