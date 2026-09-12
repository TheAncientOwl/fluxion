/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DummyLogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Dummy implementation of a LogsPlugin
///

#include <vector>

#include "Fluxion/API/LogsPlugin/Bridge.hpp"
#include "Fluxion/API/LogsPlugin/Host.hpp"
#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"
#include "Graphite/Common/Plugin/GraphiteExport.hpp"

namespace Fluxion::Plugins::Logs::DummyLogsPlugin {

namespace Bridge = Fluxion::API::LogsPlugin::Bridge;

class GRAPHITE_EXPORT DummyLogsPlugin : public Fluxion::API::LogsPlugin::IFluxionLogsPlugin
{
public:
    DummyLogsPlugin();

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
    std::vector<std::vector<std::string>> m_logs;
    std::vector<Fluxion::API::LogsPlugin::Host::LogRow> m_filtered_logs;
};

} // namespace Fluxion::Plugins::Logs::DummyLogsPlugin

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin();
typedef Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*CreateFluxionLogsPluginFactory)();
