/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DummyPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.9
/// @brief Dummy IFluxionPlugin impl with hardcoded data.
///

#include "Fluxion/API/Data.hpp"
#include "Fluxion/API/IFluxionPlugin.hpp"

#include "Graphite/Logger.hpp"

#include <algorithm>
#include <filesystem>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace Fluxion::Application {

namespace DummyImpl {

struct ComputedCondition
    : Graphite::Common::Utility::TWithFlags<ComputedCondition, Fluxion::API::Data::Filters::EConditionFlag>
{
    std::size_t column_index{};
    std::variant<std::regex, std::string> condition{};
};

struct ActiveFilter
{
    Graphite::Common::Utility::UniqueID id;
    std::uint8_t priority{};
    std::vector<ComputedCondition> conditions{};
    Fluxion::API::Data::Logs::LogRowMetadata log_row_metadata{};
};

inline std::vector<ActiveFilter> Convert(std::vector<Fluxion::API::Data::Filters::Active::Filter> filters)
{
    using namespace Fluxion::API::Data::Filters;
    LOG_INFO("SIZE: {}", filters.size());

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

        out.emplace_back(
            filter.id,
            filter.priority,
            std::move(out_conditions),
            Fluxion::API::Data::Logs::LogRowMetadata{.colors = filter.colors});
    }

    return out;
}

}; // namespace DummyImpl

class DummyPlugin : public Fluxion::API::IFluxionPlugin
{
public:
    DummyPlugin()
    {
        static const std::vector<std::string> levels = {"info", "error", "debug", "trace"};
        static const std::vector<std::string> channels = {
            "Channel1", "Channel2", "Channel3", "Channel4"};
        std::random_device rd;
        std::mt19937 gen(rd());
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

    void OnEnable(Fluxion::API::Data::Plugin::OnEnableData const& /*data*/) const override
    {
        // No action needed for dummy plugin
    }

    void OnDisable(Fluxion::API::Data::Plugin::OnDisableData const& /*data*/) const override
    {
        // No action needed for dummy plugin
    }

    std::string_view GetDisplayName() const override
    {
        static constexpr std::string_view name = "DummyPlugin";
        return name;
    }

    void RenderMenuLayer() override
    {
        // No UI to render for dummy plugin
    }

    void ImportLogs(std::filesystem::path const& /*path*/) override
    {
        // No import functionality for dummy plugin
    }

    void ApplyFilters(
        std::vector<Fluxion::API::Data::Filters::Active::Filter> _filters,
        std::vector<Fluxion::API::Data::Filters::Active::Filter> _highlight_only) override
    {
        LOG_SCOPE("");
        using namespace Fluxion::API::Data::Filters;

        auto const filters = DummyImpl::Convert(std::move(_filters));
        auto const highlight_only = DummyImpl::Convert(std::move(_highlight_only));
        LOG_DEBUG("Active filters size: {}", filters.size());
        LOG_DEBUG("HighlightOnly-Active filters size: {}", highlight_only.size());

        m_filtered_logs.clear();
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
                        Fluxion::API::Data::Logs::LogRowMetadata{
                            .colors = {filter.log_row_metadata.colors}});
                    break;
                }
            }
        }

        if (filters.empty())
        {
            DisableFilters();
        }

        for (auto& filtered_log : m_filtered_logs)
        {
            for (auto const& highlight_filter : highlight_only)
            {
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
                    filtered_log.metadata = highlight_filter.log_row_metadata;
                    break;
                }
            }
        }
    }

    void DisableFilters() override
    {
        m_filtered_logs.clear();
        for (auto const& log : m_logs)
        {
            m_filtered_logs.push_back({.data = log, .metadata = {}});
        }
    }

    std::vector<Fluxion::API::Data::Logs::ColumnDetails> GetTableHeader() const override
    {
        static std::vector<Fluxion::API::Data::Logs::ColumnDetails> s_table_header{
            {Graphite::Common::Utility::UniqueID::Generate(), "Timestamp"},
            {Graphite::Common::Utility::UniqueID::Generate(), "Channel"},
            {Graphite::Common::Utility::UniqueID::Generate(), "Level"},
            {Graphite::Common::Utility::UniqueID::Generate(), "Payload"}};
        return s_table_header;
    }

    std::size_t GetTotalLogs() const override { return m_filtered_logs.size(); }

    void GetLogs(
        std::vector<Fluxion::API::Data::Logs::Range> const& ranges,
        Fluxion::API::Data::Logs::IndexToLogRowMapWriter out_logs) const override
    {
        for (auto const& range : ranges)
        {
            if (m_filtered_logs.empty() || range.begin >= m_filtered_logs.size())
            {
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

private:
    std::vector<std::vector<std::string>> m_logs;
    std::vector<Fluxion::API::Data::Logs::LogRow> m_filtered_logs;
};

} // namespace Fluxion::Application
