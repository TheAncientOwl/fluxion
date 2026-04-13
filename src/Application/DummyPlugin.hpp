/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DummyPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.14
/// @brief Dummy IFluxionPlugin impl with hardcoded data.
///

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"
#include "Fluxion/API/LogsPlugin/PluginBridge.hpp"

#include "Graphite/Logger.hpp"

#include <algorithm>
#include <filesystem>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

DEFINE_LOG_SCOPE(Fluxion::Application::DummyPlugin);

namespace Fluxion::Application {

namespace DummyImpl {

struct ComputedCondition
    : Graphite::Common::Utility::TWithFlags<ComputedCondition, Fluxion::API::LogsPlugin::Data::EConditionFlag>
{
    std::size_t column_index{};
    std::variant<std::regex, std::string> condition{};
};

struct ActiveFilter
{
    Graphite::Common::Utility::UniqueID id;
    std::uint8_t priority{};
    std::vector<ComputedCondition> conditions{};
};

inline std::vector<ActiveFilter> Convert(std::vector<Fluxion::API::LogsPlugin::Data::Filter> filters)
{
    USE_LOG_SCOPE(Fluxion::Application::DummyPlugin);
    using namespace Fluxion::API::LogsPlugin::Data;
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
                out_condition.condition = std::move(condition.data);
            }
        }

        out.emplace_back(filter.id, filter.priority, std::move(out_conditions));
    }

    return out;
}

}; // namespace DummyImpl

class DummyPlugin : public Fluxion::API::LogsPlugin::IFluxionLogsPlugin
{
public:
    DummyPlugin()
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

    void OnEnable(Fluxion::API::LogsPlugin::Data::OnEnableData const& /*data*/) const override
    {
        // No action needed for dummy plugin
    }

    void OnDisable(Fluxion::API::LogsPlugin::Data::OnDisableData const& /*data*/) const override
    {
        // No action needed for dummy plugin
    }

    std::string_view GetDisplayName() const override { return "DummyPlugin"; }

    void RenderMenu() override
    {
        // No UI to render for dummy plugin
    }

    void ImportLogs(std::filesystem::path const& path) override
    {
        USE_LOG_SCOPE(Fluxion::Application::DummyPlugin);
        LOG_SCOPE("ImportLogs");
        LOG_INFO("::ImportLogs(): Importing {}", path.c_str());
    }

    void ApplyFilters(
        std::vector<Fluxion::API::LogsPlugin::Data::Filter> _filters,
        std::vector<Fluxion::API::LogsPlugin::Data::Filter> _highlight_only) override
    {
        USE_LOG_SCOPE(Fluxion::Application::DummyPlugin);
        LOG_SCOPE("::ApplyFilters()");
        using namespace Fluxion::API::LogsPlugin::Data;

        m_filter_to_search_log_index.clear();

        auto const filters = DummyImpl::Convert(std::move(_filters));
        auto const highlight_only = DummyImpl::Convert(std::move(_highlight_only));
        LOG_INFO("::ApplyFilters(): Active filters size: {}", filters.size());
        LOG_INFO("::ApplyFilters(): HighlightOnly-Active filters size: {}", highlight_only.size());

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
                        condition[EConditionFlag::IsRegex]
                            ? std::regex_match(target, std::get<std::regex>(condition.condition))
                            : target == std::get<std::string>(condition.condition)};

                    if (condition[EConditionFlag::IsEquals] != equals)
                    {
                        matches = false;
                        break;
                    }
                }

                if (matches)
                {
                    m_filtered_logs.emplace_back(
                        log,
                        Fluxion::API::LogsPlugin::Data::LogRowMetadata{
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
                        condition[EConditionFlag::IsRegex]
                            ? std::regex_match(target, std::get<std::regex>(condition.condition))
                            : target == std::get<std::string>(condition.condition)};

                    if (condition[EConditionFlag::IsEquals] != equals)
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

    void DisableFilters() override
    {
        m_filter_to_search_log_index.clear();

        m_filtered_logs.clear();
        for (auto const& log : m_logs)
        {
            m_filtered_logs.push_back({.data = log, .metadata = {}});
        }
    }

    std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> GetTableHeader() const override
    {
        static std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> s_table_header{
            {Graphite::Common::Utility::UniqueID::Generate(), "Timestamp"},
            {Graphite::Common::Utility::UniqueID::Generate(), "Channel"},
            {Graphite::Common::Utility::UniqueID::Generate(), "Level"},
            {Graphite::Common::Utility::UniqueID::Generate(), "Payload"}};
        return s_table_header;
    }

    std::size_t GetTotalLogs() const override { return m_filtered_logs.size(); }

    void GetLogs(
        std::vector<Fluxion::API::LogsPlugin::Data::Range> const& ranges,
        Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter out_logs) const override
    {
        USE_LOG_SCOPE(Fluxion::Application::DummyPlugin);
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

                auto& target_row = out_logs[idx];

                if (target_row.data.size() < source_row.data.size())
                {
                    target_row.data.resize(source_row.data.size());
                }

                for (std::size_t col_idx = 0; col_idx < source_row.data.size(); ++col_idx)
                {
                    target_row.data[col_idx] = source_row.data[col_idx];
                }

                target_row.metadata = source_row.metadata;
            }
        }
    }

    std::optional<std::size_t> GetNextLog(Graphite::Common::Utility::UniqueID const& filter_id) override
    {
        USE_LOG_SCOPE(Fluxion::Application::DummyPlugin);
        LOG_SCOPE("::GetNextLog()");
        auto& index = m_filter_to_search_log_index[filter_id];

        if (m_filtered_logs.empty())
        {
            LOG_INFO("::GetNextLog(): No logs to filter");
            index = std::nullopt;
            return std::nullopt;
        }

        std::size_t start = 0;
        if (static_cast<bool>(index) && *index + 1 < m_filtered_logs.size())
        {
            start = *index + 1;
        }

        for (std::size_t log_idx = start; log_idx < m_filtered_logs.size(); ++log_idx)
        {
            if (m_filtered_logs[log_idx].metadata.filter_id == filter_id ||
                m_filtered_logs[log_idx].metadata.highlight_id == filter_id)
            {
                index = log_idx;
                return log_idx;
            }
        }

        // wrap around
        for (std::size_t log_idx = 0; log_idx < start; ++log_idx)
        {
            if (m_filtered_logs[log_idx].metadata.filter_id == filter_id ||
                m_filtered_logs[log_idx].metadata.highlight_id == filter_id)
            {
                index = log_idx;
                return log_idx;
            }
        }

        index = std::nullopt;
        return std::nullopt;
    }

    std::optional<std::size_t> GetPrevLog(Graphite::Common::Utility::UniqueID const& filter_id) override
    {
        USE_LOG_SCOPE(Fluxion::Application::DummyPlugin);
        LOG_SCOPE("::GetPrevLog()");
        auto& index = m_filter_to_search_log_index[filter_id];

        if (m_filtered_logs.empty())
        {
            LOG_INFO("::GetPrevLog(): No logs to filter");
            index = std::nullopt;
            return std::nullopt;
        }

        std::size_t start;
        if (index.has_value() && *index > 0)
        {
            start = *index - 1;
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
                index = log_idx;
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
                    index = log_idx;
                    return log_idx;
                }
            }
        }

        index = std::nullopt;
        return std::nullopt;
    }

private:
    std::vector<std::vector<std::string>> m_logs;
    std::vector<Fluxion::API::LogsPlugin::Data::LogRow> m_filtered_logs;

    std::unordered_map<Graphite::Common::Utility::UniqueID, std::optional<std::size_t>>
        m_filter_to_search_log_index{};
};

} // namespace Fluxion::Application
