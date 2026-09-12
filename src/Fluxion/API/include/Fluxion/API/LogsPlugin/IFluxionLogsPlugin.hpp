/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file IFluxionLogsPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 2.0
/// @brief Plugin interface of Fluxion logs logic (parse/select/filter/...).
///

#pragma once

#include <optional>
#include <string_view>
#include <vector>

#include "Fluxion/API/LogsPlugin/Bridge.hpp"
#include "Fluxion/API/LogsPlugin/Host.hpp"
#include "Graphite/Common/Plugin/GraphiteExport.hpp"

namespace Fluxion::API::LogsPlugin {

class IFluxionLogsPlugin
{
protected:
    // ---------------------------------------------------------
    // ABI-SAFE BOUNDARY (Pure virtuals using raw pointers/PODs)
    // ---------------------------------------------------------

    ///
    /// @brief ABI boundary method for getting the display name.
    ///
    /// @return A zero-copy string view containing the display name.
    ///
    virtual Bridge::ABI::StringView GetDisplayNameABI() const = 0;

    ///
    /// @brief ABI boundary method for getting the directory name.
    ///
    /// @return A zero-copy string view containing the directory name.
    ///
    virtual Bridge::ABI::StringView GetDirectoryNameABI() const = 0;

    ///
    /// @brief Import log files into the plugin from a specified target path.
    ///
    /// @param [in] path UTF-8 encoded path to the log source file or directory.
    ///
    virtual void ImportLogsABI(Bridge::ABI::StringView const path) = 0;

    ///
    /// @brief ABI boundary method to retrieve table header details.
    ///
    /// @return An ABI span containing column detail definitions.
    ///
    virtual Bridge::ABI::Span<Bridge::ColumnDetails> GetTableHeaderABI() const = 0;

    ///
    /// @brief ABI boundary method to request specific log chunks.
    ///
    /// @param [in] ranges Span of requested index ranges (inclusive begin, exclusive end).
    /// @param [out] out_logs Writer interface to push log content directly into host memory.
    ///
    virtual void GetLogsABI(Bridge::ABI::Span<Bridge::Range> const ranges, Bridge::ILogsWriter* out_logs) = 0;

    ///
    /// @brief ABI boundary method for applying log filters.
    ///
    /// @param [in] filters Span of active filters to display.
    /// @param highlight_only Span of filters used purely for visual highlighting.
    ///
    virtual void ApplyFiltersABI(
        Bridge::ABI::Span<Bridge::Filter const> const filters,
        Bridge::ABI::Span<Bridge::Filter const> const highlight_only) = 0;

    ///
    /// @brief ABI boundary method to find the next matching log index.
    ///
    /// @param [in] filter_id Unique identifier of the filter context.
    /// @param [in] current_index Current selected item index as a starting reference.
    /// @param [out] out_index Pointer to store the resulting matched index if found.
    ///
    /// @return true if a next log was found, false otherwise.
    ///
    virtual bool GetNextLogABI(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index,
        std::size_t* out_index) = 0;

    ///
    /// @brief ABI boundary method to find the previous matching log index.
    ///
    /// @param [in] filter_id Unique identifier of the filter context.
    /// @param [in] current_index Current selected item index as a starting reference.
    /// @param [out] out_index Pointer to store the resulting matched index if found.
    ///
    /// @return true if a previous log was found, false otherwise.
    ///
    virtual bool GetPrevLogABI(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index,
        std::size_t* out_index) = 0;

public:
    ///
    /// @brief Virtual destructor.
    ///
    virtual ~IFluxionLogsPlugin() = default;

    ///
    /// @brief Lifecycle callback invoked when the plugin is enabled.
    ///
    /// @param [in] data Initialization data structure containing home paths.
    ///
    virtual void OnEnable(Bridge::OnEnableData const& data) = 0;

    ///
    /// @brief Lifecycle callback invoked when the plugin is disabled.
    ///
    /// @param [in] data Shutdown data structure.
    ///
    virtual void OnDisable(Bridge::OnDisableData const& data) = 0;

    ///
    /// @brief Render plugin-specific ImGui menu items or settings panels.
    ///
    virtual void RenderMenu() = 0;

    ///
    /// @brief Clear or mark all currently active filters as disabled.
    ///
    virtual void DisableFilters() = 0;

    ///
    /// @brief Get the total count of currently filtered logs available.
    ///
    /// @return The total number of filtered log entries.
    ///
    virtual std::size_t GetTotalLogs() const = 0;

    ///
    /// @brief Get the total target metric for ongoing background log operations.
    ///
    /// @return Target count required to reach 100% completion.
    ///
    virtual std::size_t GetLogsOperationTarget() const = 0;

    ///
    /// @brief Get the unit type used to measure log operation progress (e.g., logs or bytes).
    ///
    /// @return The operation unit type.
    ///
    virtual Bridge::ELogsOperationUnit GetLogsOperationUnit() const = 0;

    ///
    /// @brief Track how many units have been processed at the time of the call.
    ///
    /// @return Current progress value.
    ///
    virtual std::size_t GetLogsOperationProgress() const = 0;

public:
    // ---------------------------------------------------------
    // INLINE C++ DEV ADAPTERS
    // ---------------------------------------------------------

    ///
    /// @brief Get the display name of the plugin.
    ///
    /// @return A string view of the display name.
    ///
    inline std::string_view GetDisplayName() const { return GetDisplayNameABI(); }

    ///
    /// @brief Get the directory name associated with the plugin.
    ///
    /// @return A string view of the directory name.
    ///
    inline std::string_view GetDirectoryName() const { return GetDirectoryNameABI(); }

    ///
    /// @brief Import log files into the plugin from a specified target path.
    ///
    /// @param [in] path UTF-8 encoded path to the log source file or directory.
    ///
    inline void ImportLogs(std::filesystem::path const& path)
    {
        ImportLogsABI(std::string_view(path.string()));
    }

    ///
    /// @brief Apply active filters and visual highlights via standard spans.
    ///
    /// @param [in] filters Span of active filters.
    /// @param [in] highlight_only Span of highlight-only filters.
    ///
    inline void ApplyFilters(
        std::span<Bridge::Filter const> const filters,
        std::span<Bridge::Filter const> const highlight_only)
    {
        ApplyFiltersABI(filters, highlight_only);
    }

    ///
    /// @brief Get the next log index relative to the current item.
    ///
    /// @param [in] filter_id The filter ID to search within.
    /// @param [in] current_index Current selected log index reference point.
    ///
    /// @return An optional containing the next log index, or std::nullopt if not found.
    ///
    inline std::optional<std::size_t> GetNextLog(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index = 0)
    {
        std::size_t idx{};
        if (GetNextLogABI(filter_id, current_index, &idx))
        {
            return idx;
        }
        return std::nullopt;
    }

    ///
    /// @brief Get the previous log index relative to the current item.
    ///
    /// @param [in] filter_id The filter ID to search within.
    /// @param [in] current_index Current selected log index reference point.
    ///
    /// @return An optional containing the previous log index, or std::nullopt if not found.
    ///
    inline std::optional<std::size_t> GetPrevLog(
        Graphite::Common::Utility::UniqueID const& filter_id,
        std::size_t const current_index = 0)
    {
        std::size_t idx{};
        if (GetPrevLogABI(filter_id, current_index, &idx))
        {
            return idx;
        }
        return std::nullopt;
    }

    ///
    /// @brief Get table header configuration details.
    ///
    /// @return A span of column details.
    ///
    inline std::vector<Host::ColumnDetails> GetTableHeader() const
    {
        std::vector<Host::ColumnDetails> out{};
        std::span<const Bridge::ColumnDetails> const header{GetTableHeaderABI()};
        out.reserve(header.size());

        return std::vector<Host::ColumnDetails>{header.begin(), header.end()};
    }

    ///
    /// @brief Request log chunks using index ranges and a target writer.
    ///
    /// @param [in] ranges Vector of requested ranges.
    /// @param [out] out_logs Log writer target interface.
    ///
    inline void GetLogs(std::vector<Bridge::Range> const& ranges, Bridge::ILogsWriter* out_logs)
    {
        GetLogsABI(ranges, out_logs);
    }
};

} // namespace Fluxion::API::LogsPlugin

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin();
typedef Fluxion::API::LogsPlugin::IFluxionLogsPlugin* (*CreateFluxionLogsPluginFactory)();
