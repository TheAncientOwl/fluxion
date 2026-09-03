/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RegexTags.hpp
/// @author Alexandru Delegeanu
/// @version 9.0
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#pragma once

#include <memory>
#include <vector>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"
#include "Graphite/Common/DataStructures/TDoubleBuffer.hpp"
#include "Graphite/Common/Plugin/GraphiteExport.hpp"
#include "Graphite/Settings/PersistentSettings.hpp"
#include "SQLite/SQLiteStorage.hpp"

#include "Data.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V9 {

class GRAPHITE_EXPORT RegexTags : public Fluxion::API::LogsPlugin::IFluxionLogsPlugin
{
public:
    RegexTags();
    ~RegexTags() override;

    std::string_view GetDisplayName() const override final;
    std::string_view GetDirectoryName() const override final;

    void OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& data) override final;
    void OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& data) override final;

    void RenderMenu() override final;

    void ImportLogs(std::filesystem::path const& path) override final;

    std::optional<std::size_t> GetNextLog(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index) override final;
    std::optional<std::size_t> GetPrevLog(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index) override final;

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

private:
    std::filesystem::path MakeDatabasePath(std::filesystem::path const& raw_logs_path) const;
    Graphite::Settings::PersistentSettings GetConfig() const;

    void SaveRegexTags(std::vector<std::shared_ptr<Data::RegexTag>> const& tags) const;
    std::vector<std::shared_ptr<Data::RegexTag>> LoadRegexTags() const;
    void UpdateImportedLogsHeader(std::vector<std::shared_ptr<Data::RegexTag>> const& tags);
    Fluxion::API::LogsPlugin::Data::ELogsOperationUnit GetLogsOperationUnit() const override final;

    void LoadSettings();
    void SaveSettings() const;

private:
    Graphite::Common::DataStructures::TCopyDoubleBuffer<std::vector<std::shared_ptr<Data::RegexTag>>>
        m_regex_tags{};

    std::filesystem::path m_home_path{};
    std::optional<std::filesystem::path> m_last_imported_logs_path{};
    std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> m_imported_logs_header{};
    std::atomic<std::size_t> m_logs_operation_progress{0};
    std::size_t m_logs_operation_target{0};
    Fluxion::API::LogsPlugin::Data::ELogsOperationUnit m_logs_operation_unit{
        Fluxion::API::LogsPlugin::Data::ELogsOperationUnit::Logs};

    std::vector<std::unique_ptr<SQLiteStorage>> m_sqlite_storages{};
    std::vector<Data::FilteredLog> m_filtered_logs{};
    std::size_t m_total_logs_imported{0};

    Data::Settings m_settings{};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V9

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin();
typedef Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*CreateFluxionLogsPluginFactory)();
