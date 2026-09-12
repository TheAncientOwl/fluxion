/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DummyLogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Implementation of @see DummyLogsPlugin.hpp
///

#include <algorithm>
#include <filesystem>
#include <random>
#include <regex>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "Fluxion/Plugins/Logs/DummyPlugin/DummyLogsPlugin.hpp"
#include "Graphite/Common/UI/ImGuiHelpers.hpp"
#include "Graphite/Common/Utility/TWithFlags.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::DummyLogsPlugin);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::DummyLogsPlugin);

namespace Fluxion::Plugins::Logs::DummyLogsPlugin {

namespace DummyImpl {

struct ComputedCondition
    : Graphite::Common::Utility::TWithFlags<ComputedCondition, Fluxion::API::LogsPlugin::Bridge::EConditionFlag>
{
    using TWithFlags<ComputedCondition, Fluxion::API::LogsPlugin::Bridge::EConditionFlag>::operator[];

    std::size_t column_index{};
    std::variant<std::regex, std::string> condition{};
};

struct ActiveFilter
{
    Graphite::Common::Utility::UniqueID id;
    std::uint8_t priority{};
    std::vector<ComputedCondition> conditions{};
};

inline std::vector<ActiveFilter> Convert(std::span<Bridge::Filter const> const filters)
{
    LOG_INFO("::DummyImpl::Convert(): SIZE: {}", filters.size());

    std::vector<ActiveFilter> out{};
    out.reserve(filters.size());

    for (std::size_t i = 0; i < filters.size(); ++i)
    {
        auto const& filter = filters[i];
        std::vector<ComputedCondition> out_conditions{};
        std::span<Bridge::Condition const> conditions_span{filter.conditions};
        out_conditions.reserve(conditions_span.size());

        for (std::size_t j = 0; j < conditions_span.size(); ++j)
        {
            auto const& condition = conditions_span[j];
            auto& out_condition = out_conditions.emplace_back();
            out_condition.column_index = condition.column_index;

            out_condition[Bridge::EConditionFlag::IsRegex] =
                condition[Bridge::EConditionFlag::IsRegex];
            out_condition[Bridge::EConditionFlag::IsEquals] =
                condition[Bridge::EConditionFlag::IsEquals];
            out_condition[Bridge::EConditionFlag::IsCaseSensitive] =
                condition[Bridge::EConditionFlag::IsCaseSensitive];

            std::string_view const cond_data_sv(condition.data.data, condition.data.size);
            if (condition[Bridge::EConditionFlag::IsRegex])
            {
                out_condition.condition = std::regex{std::string(cond_data_sv)};
            }
            else
            {
                out_condition.condition = std::string(cond_data_sv);
            }
        }

        out.emplace_back(filter.id, filter.priority, std::move(out_conditions));
    }

    return out;
}

}; // namespace DummyImpl

DummyLogsPlugin::DummyLogsPlugin()
{
    static const std::vector<std::string> levels = {"info", "error", "debug", "trace"};
    static const std::vector<std::string> channels = {
        "Channel1", "Channel2", "Channel3", "Channel4"};
    constexpr std::uint32_t seed = 69420;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> level_dist(0, static_cast<int>(levels.size() - 1));
    std::uniform_int_distribution<> channel_dist(0, static_cast<int>(channels.size() - 1));

    for (std::size_t i = 0; i < 1000; ++i)
    {
        std::vector<std::string> entry{};
        entry.push_back(std::string("2026-01-01 12:00:") + (i < 10 ? "0" : "") + std::to_string(i));
        auto const channel_idx{static_cast<std::size_t>(std::max(0, channel_dist(gen)))};
        entry.push_back(channels[channel_idx]);
        auto const level_idx{static_cast<std::size_t>(std::max(0, level_dist(gen)))};
        entry.push_back(levels[level_idx]);
        entry.push_back(
            "Dummy log entry number " + std::to_string(i) + " ---------------------------");
        m_logs.push_back(std::move(entry));
        m_filtered_logs.push_back({m_logs.back(), {}});
    }
}

void DummyLogsPlugin::OnEnable(Bridge::OnEnableData const& /*data*/)
{
    // No action needed for dummy plugin
}

void DummyLogsPlugin::OnDisable(Bridge::OnDisableData const& /*data*/)
{
    // No action needed for dummy plugin
}

Bridge::ABI::StringView DummyLogsPlugin::GetDisplayNameABI() const
{
    return "DummyLogsPlugin";
}

Bridge::ABI::StringView DummyLogsPlugin::GetDirectoryNameABI() const
{
    return "DummyLogsPlugin";
}

void DummyLogsPlugin::RenderMenu()
{
    // No UI to render for dummy plugin
}

void DummyLogsPlugin::ImportLogsABI(Bridge::ABI::StringView const path_view)
{
    LOG_SCOPE("ImportLogsABI");
    std::filesystem::path const path{std::string_view(path_view.data, path_view.size)};
    LOG_INFO("::ImportLogsABI(): Importing {}", path.string());
}

void DummyLogsPlugin::ApplyFiltersABI(
    Bridge::ABI::Span<Bridge::Filter const> const _filters,
    Bridge::ABI::Span<Bridge::Filter const> const _highlight_only)
{
    LOG_SCOPE("::ApplyFiltersABI()");

    auto const filters = DummyImpl::Convert(_filters);
    auto const highlight_only = DummyImpl::Convert(_highlight_only);
    LOG_INFO("::ApplyFiltersABI(): Active filters size: {}", filters.size());
    LOG_INFO("::ApplyFiltersABI(): HighlightOnly-Active filters size: {}", highlight_only.size());

    m_filtered_logs.clear();
    std::vector<std::uint8_t> priorities{};
    for (auto const& log : m_logs)
    {
        for (auto const& filter : filters)
        {
            bool matches{true};
            for (auto const& condition : filter.conditions)
            {
                auto const& target{log[condition.column_index]};

                bool const equals{
                    condition[Bridge::EConditionFlag::IsRegex]
                        ? std::regex_match(target, std::get<std::regex>(condition.condition))
                        : target == std::get<std::string>(condition.condition)};

                if (condition[Bridge::EConditionFlag::IsEquals] != equals)
                {
                    matches = false;
                    break;
                }
            }

            if (matches)
            {
                m_filtered_logs.emplace_back(
                    log,
                    Fluxion::API::LogsPlugin::Bridge::LogRowMetadata{
                        .filter_id = filter.id, .highlight_id = filter.id});
                priorities.push_back(filter.priority);
                break;
            }
        }
    }

    if (filters.empty())
    {
        DisableFilters();
        priorities.clear();
        priorities.resize(m_filtered_logs.size());
        std::fill(priorities.begin(), priorities.end(), 0);
    }

    std::size_t idx{0};
    for (auto& filtered_log : m_filtered_logs)
    {
        for (auto const& highlight_filter : highlight_only)
        {
            if (highlight_filter.priority < priorities[idx])
            {
                continue;
            }

            bool matches{true};
            for (auto const& condition : highlight_filter.conditions)
            {
                auto const& target{filtered_log.data[condition.column_index]};

                bool const equals{
                    condition[Bridge::EConditionFlag::IsRegex]
                        ? std::regex_match(target, std::get<std::regex>(condition.condition))
                        : target == std::get<std::string>(condition.condition)};

                if (condition[Bridge::EConditionFlag::IsEquals] != equals)
                {
                    matches = false;
                    break;
                }
            }

            if (matches)
            {
                filtered_log.metadata.highlight_id = highlight_filter.id;
                break;
            }
        }
        ++idx;
    }
}

void DummyLogsPlugin::DisableFilters()
{
    m_filtered_logs.clear();
    for (auto const& log : m_logs)
    {
        m_filtered_logs.push_back({.data = log, .metadata = {}});
    }
}

Bridge::ABI::Span<Bridge::ColumnDetails> DummyLogsPlugin::GetTableHeaderABI() const
{
    static std::vector<Bridge::ColumnDetails> s_table_header{
        {Graphite::Common::Utility::UniqueID::Generate(), Bridge::ABI::StringView{"Timestamp"}},
        {Graphite::Common::Utility::UniqueID::Generate(), Bridge::ABI::StringView{"Channel"}},
        {Graphite::Common::Utility::UniqueID::Generate(), Bridge::ABI::StringView{"Level"}},
        {Graphite::Common::Utility::UniqueID::Generate(), Bridge::ABI::StringView{"Payload"}}};
    return s_table_header;
}

std::size_t DummyLogsPlugin::GetTotalLogs() const
{
    return m_filtered_logs.size();
}

void DummyLogsPlugin::GetLogsABI(Bridge::ABI::Span<Bridge::Range> const _ranges, Bridge::ILogsWriter* out_logs)
{
    LOG_SCOPE("::GetLogsABI()");

    if (!out_logs)
    {
        return;
    }

    std::span<Bridge::Range const> ranges{_ranges};

    for (std::size_t range_idx = 0; range_idx < ranges.size(); ++range_idx)
    {
        auto const& range = ranges[range_idx];
        LOG_TRACE("::GetLogsABI(): begin {} | end {}", range.begin, range.end);

        if (m_filtered_logs.empty() || range.begin >= m_filtered_logs.size())
        {
            LOG_TRACE(
                "::GetLogsABI(): empty == {} | begin over size == {}",
                m_filtered_logs.empty(),
                range.begin >= m_filtered_logs.size());
            continue;
        }

        auto const end_idx = std::min(range.end, m_filtered_logs.size());

        for (std::size_t idx = range.begin; idx < end_idx; ++idx)
        {
            auto const& source_row = m_filtered_logs[idx];

            // Build temporary vector of ABI::StringView for columns
            std::vector<Bridge::ABI::StringView> column_views;
            column_views.reserve(source_row.data.size());
            for (auto const& col : source_row.data)
            {
                column_views.emplace_back(col);
            }

            out_logs->WriteData(idx, column_views);
            out_logs->WriteMetadata(
                idx, source_row.metadata.filter_id, source_row.metadata.highlight_id);
        }
    }
}

bool DummyLogsPlugin::GetNextLogABI(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index,
    std::size_t* out_index)
{
    LOG_SCOPE("::GetNextLogABI()");

    if (m_filtered_logs.empty() || !out_index)
    {
        LOG_INFO("::GetNextLogABI(): No logs to filter or null out_index");
        return false;
    }

    std::size_t start = current_index + 1;
    if (start >= m_filtered_logs.size())
    {
        start = 0; // Wrap around
    }

    for (std::size_t log_idx = start; log_idx < m_filtered_logs.size(); ++log_idx)
    {
        if (m_filtered_logs[log_idx].metadata.filter_id == filter_id ||
            m_filtered_logs[log_idx].metadata.highlight_id == filter_id)
        {
            *out_index = log_idx;
            return true;
        }
    }

    // wrap around
    for (std::size_t log_idx = 0; log_idx < start; ++log_idx)
    {
        if (m_filtered_logs[log_idx].metadata.filter_id == filter_id ||
            m_filtered_logs[log_idx].metadata.highlight_id == filter_id)
        {
            *out_index = log_idx;
            return true;
        }
    }

    return false;
}

bool DummyLogsPlugin::GetPrevLogABI(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index,
    std::size_t* out_index)
{
    LOG_SCOPE("::GetPrevLogABI()");

    if (m_filtered_logs.empty() || !out_index)
    {
        LOG_INFO("::GetPrevLogABI(): No logs to filter or null out_index");
        return false;
    }

    std::size_t start;
    if (current_index > 0)
    {
        start = current_index - 1;
    }
    else
    {
        start = m_filtered_logs.size() - 1; // wrap to end
    }

    // backward search
    for (std::size_t log_idx = start + 1; log_idx-- > 0;)
    {
        if (m_filtered_logs[log_idx].metadata.filter_id == filter_id ||
            m_filtered_logs[log_idx].metadata.highlight_id == filter_id)
        {
            *out_index = log_idx;
            return true;
        }
    }

    // wrap around (search from end to original position)
    if (start != m_filtered_logs.size() - 1)
    {
        for (std::size_t log_idx = m_filtered_logs.size(); log_idx-- > start + 1;)
        {
            if (m_filtered_logs[log_idx].metadata.filter_id == filter_id ||
                m_filtered_logs[log_idx].metadata.highlight_id == filter_id)
            {
                *out_index = log_idx;
                return true;
            }
        }
    }

    return false;
}

std::size_t DummyLogsPlugin::GetLogsOperationTarget() const
{
    return 0;
}

std::size_t DummyLogsPlugin::GetLogsOperationProgress() const
{
    return 0;
}

Bridge::ELogsOperationUnit DummyLogsPlugin::GetLogsOperationUnit() const
{
    return Bridge::ELogsOperationUnit::Logs;
}

} // namespace Fluxion::Plugins::Logs::DummyLogsPlugin

extern "C" GRAPHITE_EXPORT Fluxion::API::LogsPlugin::IFluxionLogsPlugin* CreateFluxionLogsPlugin()
{
    return new Fluxion::Plugins::Logs::DummyLogsPlugin::DummyLogsPlugin();
}
