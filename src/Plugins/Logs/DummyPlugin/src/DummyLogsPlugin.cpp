/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
///
/// @file DummyLogsPlugin.cpp
/// @author Alexandru Delegeanu
/// @version 0.13
/// @brief Implementation of @see DummyLogsPlugin.hpp
///
/// --------------------------------------------------------------------------

#include <algorithm>
#include <filesystem>
#include <random>
#include <regex>
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

FLUXION_REGISTER_LOGS_PLUGIN(Fluxion::LogsPlugin::DummyLogsPlugin::DummyLogsPlugin);

namespace Fluxion::LogsPlugin::DummyLogsPlugin {

namespace DummyImpl {

struct ComputedCondition
    : Graphite::Common::Utility::TWithFlags<ComputedCondition, Fluxion::API::LogsPlugin::EConditionFlag>
{
    using TWithFlags<ComputedCondition, Fluxion::API::LogsPlugin::EConditionFlag>::operator[];

    std::size_t column_index{};
    std::variant<std::regex, std::string> condition{};
};

struct ActiveFilter
{
    Fluxion::API::LogsPlugin::UniqueID id;
    std::uint8_t priority{};
    std::vector<ComputedCondition> conditions{};
};

///
/// @note Conversion has to be done because of plugin specific regex implementation
/// TODO: Consider moving this on Fluxion side with a callback / template type for regex handling.
///

inline std::vector<ActiveFilter> Convert(std::span<Fluxion::API::LogsPlugin::Filter const> filters)
{
    using EConditionFlag = Fluxion::API::LogsPlugin::EConditionFlag;

    LOG_INFO("::DummyImpl::Convert(): SIZE: {}", filters.size());

    std::vector<ActiveFilter> out{};
    out.reserve(filters.size());

    for (auto const& filter : filters)
    {
        std::vector<ComputedCondition> out_conditions{};
        out_conditions.reserve(filter.conditions.size());

        for (auto const& condition : filter.conditions)
        {
            auto& out_condition = out_conditions.emplace_back();

            out_condition.column_index = condition.column_index;

            out_condition[EConditionFlag::IsRegex] = condition[EConditionFlag::IsRegex];

            out_condition[EConditionFlag::IsEquals] = condition[EConditionFlag::IsEquals];

            out_condition[EConditionFlag::IsCaseSensitive] =
                condition[EConditionFlag::IsCaseSensitive];

            if (condition[EConditionFlag::IsRegex])
            {
                out_condition.condition = std::regex{condition.data};
            }
            else
            {
                out_condition.condition = condition.data;
            }
        }

        out.emplace_back(filter.id, filter.priority, std::move(out_conditions));
    }

    return out;
}

} // namespace DummyImpl

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

void DummyLogsPlugin::OnEnable(Fluxion::API::LogsPlugin::OnEnableData const& /*data*/)
{
    // No action needed for dummy plugin
}

void DummyLogsPlugin::OnDisable(Fluxion::API::LogsPlugin::OnDisableData const& /*data*/)
{
    // No action needed for dummy plugin
}

std::string_view DummyLogsPlugin::GetDisplayName() const
{
    return "DummyLogsPlugin";
}

std::string_view DummyLogsPlugin::GetDirectoryName() const
{
    return "DummyLogsPlugin";
}

void DummyLogsPlugin::RenderMenu()
{
    // No UI to render for dummy plugin
}

void DummyLogsPlugin::ImportLogs(std::filesystem::path const& path)
{
    LOG_SCOPE("ImportLogs");

    LOG_INFO("::ImportLogs(): Importing {}", path);
}

void DummyLogsPlugin::ApplyFilters(
    std::span<Fluxion::API::LogsPlugin::Filter const> const filters,
    std::span<Fluxion::API::LogsPlugin::Filter const> const highlight_only)
{
    LOG_SCOPE("::ApplyFilters()");

    auto const active_filters = DummyImpl::Convert(filters);
    auto const active_highlight_only = DummyImpl::Convert(highlight_only);

    LOG_INFO("::ApplyFilters(): Active filters size: {}", active_filters.size());

    LOG_INFO("::ApplyFilters(): HighlightOnly-Active filters size: {}", active_highlight_only.size());

    m_filtered_logs.clear();

    std::vector<std::uint8_t> priorities{};

    for (auto const& log : m_logs)
    {
        for (auto const& filter : active_filters)
        {
            bool matches{true};

            for (auto const& condition : filter.conditions)
            {
                auto const& target{log[condition.column_index]};

                bool const equals{
                    condition[Fluxion::API::LogsPlugin::EConditionFlag::IsRegex]
                        ? std::regex_match(target, std::get<std::regex>(condition.condition))
                        : target == std::get<std::string>(condition.condition)};

                if (condition[Fluxion::API::LogsPlugin::EConditionFlag::IsEquals] != equals)
                {
                    matches = false;
                    break;
                }
            }

            if (matches)
            {
                m_filtered_logs.emplace_back(
                    log,
                    Fluxion::API::LogsPlugin::OwningLogRowMetadata{
                        .filter_id = filter.id, .highlight_id = filter.id});

                priorities.push_back(filter.priority);
                break;
            }
        }
    }

    if (active_filters.empty())
    {
        DisableFilters();

        priorities.clear();
        priorities.resize(m_filtered_logs.size());

        std::fill(priorities.begin(), priorities.end(), 0);
    }

    std::size_t idx{0};

    for (auto& filtered_log : m_filtered_logs)
    {
        for (auto const& highlight_filter : active_highlight_only)
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
                    condition[Fluxion::API::LogsPlugin::EConditionFlag::IsRegex]
                        ? std::regex_match(target, std::get<std::regex>(condition.condition))
                        : target == std::get<std::string>(condition.condition)};

                if (condition[Fluxion::API::LogsPlugin::EConditionFlag::IsEquals] != equals)
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

std::span<Fluxion::API::LogsPlugin::ColumnDetails const> DummyLogsPlugin::GetTableHeader() const
{
    static std::vector<Fluxion::API::LogsPlugin::ColumnDetails> s_table_header{
        {Fluxion::API::LogsPlugin::UniqueID::Generate(), "Timestamp"},
        {Fluxion::API::LogsPlugin::UniqueID::Generate(), "Channel"},
        {Fluxion::API::LogsPlugin::UniqueID::Generate(), "Level"},
        {Fluxion::API::LogsPlugin::UniqueID::Generate(), "Payload"}};

    return s_table_header;
}

std::size_t DummyLogsPlugin::GetTotalLogs() const
{
    return m_filtered_logs.size();
}

void DummyLogsPlugin::GetLogs(
    std::span<Fluxion::API::LogsPlugin::Range const> const ranges,
    Fluxion::API::LogsPlugin::WriteLogRowDataFn write_data,
    Fluxion::API::LogsPlugin::WriteLogRowMetadataFn write_metadata,
    void* user_data)
{
    LOG_SCOPE("::GetLogs()");

    for (auto const& range : ranges)
    {
        LOG_TRACE("::GetLogs(): begin {} | end {}", range.begin, range.end);

        if (m_filtered_logs.empty() || range.begin >= m_filtered_logs.size())
        {
            LOG_TRACE(
                "::GetLogs(): empty == {} | begin over size == {}",
                m_filtered_logs.empty(),
                range.begin >= m_filtered_logs.size());

            continue;
        }

        auto const end_idx = std::min(range.end, m_filtered_logs.size());

        for (std::size_t idx = range.begin; idx < end_idx; ++idx)
        {
            auto const& source_row = m_filtered_logs[idx];

            std::vector<Fluxion::API::LogsPlugin::LogRowItem> row_data{};
            row_data.reserve(source_row.data.size());

            for (auto const& value : source_row.data)
            {
                row_data.push_back({.data = value.data(), .size = value.size()});
            }

            Fluxion::API::LogsPlugin::LogRowData const safe_data{
                .data = row_data.data(), .size = row_data.size()};
            GRAPHITE_ASSERT(write_data != nullptr, "Received nullptr write_data function pointer");
            write_data(user_data, idx, &safe_data);

            auto const metadata{Fluxion::API::LogsPlugin::Adapter::MakeMetadata(
                source_row.metadata.filter_id, source_row.metadata.highlight_id)};
            GRAPHITE_ASSERT(
                write_metadata != nullptr, "Received nullptr write_metadata function pointer");
            write_metadata(user_data, idx, &metadata);
        }
    }
}

std::optional<std::size_t> DummyLogsPlugin::GetNextLog(
    Fluxion::API::LogsPlugin::UniqueID const& filter_id,
    std::size_t const current_index)
{
    LOG_SCOPE("::GetNextLog()");

    if (m_filtered_logs.empty())
    {
        LOG_INFO("::GetNextLog(): No logs to filter");
        return std::nullopt;
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
            return log_idx;
        }
    }

    // wrap around
    for (std::size_t log_idx = 0; log_idx < start; ++log_idx)
    {
        if (m_filtered_logs[log_idx].metadata.filter_id == filter_id ||
            m_filtered_logs[log_idx].metadata.highlight_id == filter_id)
        {
            return log_idx;
        }
    }

    return std::nullopt;
}

std::optional<std::size_t> DummyLogsPlugin::GetPrevLog(
    Fluxion::API::LogsPlugin::UniqueID const& filter_id,
    std::size_t const current_index)
{
    LOG_SCOPE("::GetPrevLog()");

    if (m_filtered_logs.empty())
    {
        LOG_INFO("::GetPrevLog(): No logs to filter");
        return std::nullopt;
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
            return log_idx;
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
                return log_idx;
            }
        }
    }

    return std::nullopt;
}

std::size_t DummyLogsPlugin::GetLogsOperationTarget() const
{
    return 0;
}

std::size_t DummyLogsPlugin::GetLogsOperationProgress() const
{
    return 0;
}

Fluxion::API::LogsPlugin::ELogsOperationUnit DummyLogsPlugin::GetLogsOperationUnit() const
{
    return Fluxion::API::LogsPlugin::ELogsOperationUnit::Logs;
}

} // namespace Fluxion::LogsPlugin::DummyLogsPlugin
