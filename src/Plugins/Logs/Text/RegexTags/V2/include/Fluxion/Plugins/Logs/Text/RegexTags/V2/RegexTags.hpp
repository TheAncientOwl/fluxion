/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file RegexTags.hpp
/// @author Alexandru Delegeanu
/// @version 2.2.0
/// @brief Use regex to split log txt line to columns. Store data to flat files
///

#include <memory>
#include <vector>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"
#include "Graphite/Common/DataStructures/TDoubleBuffer.hpp"
#include "Graphite/Common/Plugin/GraphiteExport.hpp"
#include "Graphite/Settings/PersistentSettings.hpp"

#include "Data.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V2 {

namespace Bridge = Fluxion::API::LogsPlugin::Bridge;

class GRAPHITE_EXPORT RegexTags final : public Fluxion::API::LogsPlugin::IFluxionLogsPlugin
{
protected:
    Bridge::ABI::StringView GetDisplayNameABI() const override;
    Bridge::ABI::StringView GetDirectoryNameABI() const override;

    void ImportLogsABI(Bridge::ABI::StringView const path) override;

    Bridge::ABI::Span<Bridge::ColumnDetails> GetTableHeaderABI() const override;

    void GetLogsABI(Bridge::ABI::Span<Bridge::Range> const ranges, Bridge::ILogsWriter* out_logs) override;

    void ApplyFiltersABI(
        Bridge::ABI::Span<Bridge::Filter const> const filters,
        Bridge::ABI::Span<Bridge::Filter const> const highlight_only) override;

    bool GetNextLogABI(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index,
        std::size_t* out_index) override;

    bool GetPrevLogABI(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index,
        std::size_t* out_index) override;

public:
    void OnEnable(Bridge::OnEnableData const& data) override;
    void OnDisable(Bridge::OnDisableData const& data) override;

    void RenderMenu() override;

    void DisableFilters() override;

    std::size_t GetTotalLogs() const override;

    std::size_t GetLogsOperationTarget() const override;
    std::size_t GetLogsOperationProgress() const override;
    Bridge::ELogsOperationUnit GetLogsOperationUnit() const override;

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
    std::vector<Fluxion::API::LogsPlugin::Bridge::ColumnDetails> m_imported_logs_header{};
    std::size_t m_logs_operation_progress{0};
    std::size_t m_logs_operation_target{0};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V2

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin();
typedef Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*CreateFluxionLogsPluginFactory)();
