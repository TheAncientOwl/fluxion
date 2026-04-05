/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DummyPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.8
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

struct FilteredLogRow
{
    std::vector<std::string> row;
    Fluxion::API::Data::Logs::LogRowMetadata metadata{};
};

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

inline std::vector<ActiveFilter> ComputeActiveFilters(
    std::vector<Fluxion::API::Data::Filters::Tab::Ptr> const& tabs,
    std::vector<Fluxion::API::Data::Logs::ColumnDetails> const& header)
{
    using namespace Fluxion::API::Data::Filters;
    std::vector<ActiveFilter> out{};

    auto get_column_index = [&header](Graphite::Common::Utility::UniqueID const id) {
        for (std::size_t index = 0; index < header.size(); ++index)
        {
            if (header[index].id == id)
            {
                return index;
            }
        }

        GRAPHITE_ASSERT(false, std::string{"Failed to find header with ID "} + id);

        return std::size_t{0};
    };

    for (auto const& tab_ptr : tabs)
    {
        auto const& tab{*tab_ptr};
        if (!tab[ETabFlag::IsActive])
        {
            continue;
        }

        for (auto const& filter_ptr : tab.filters.GetBack())
        {
            auto const& filter{*filter_ptr};
            if (!filter[EFilterFlag::IsActive])
            {
                continue;
            }

            std::vector<ComputedCondition> out_conditions{};
            for (auto const& condition_ptr : filter.conditions.GetBack())
            {
                auto const& condition{*condition_ptr};
                auto& out_condition = out_conditions.emplace_back();
                out_condition.column_index = get_column_index(condition.over_column_id);

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

            if (!out_conditions.empty())
            {
                out.emplace_back(
                    filter.id,
                    filter.priority,
                    std::move(out_conditions),
                    Fluxion::API::Data::Logs::LogRowMetadata{.colors = filter.colors});
            }
        }
    }

    std::sort(
        out.begin(), out.end(), [](auto const& a, auto const& b) { return a.priority > b.priority; });

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

    void ApplyFilters(std::vector<Fluxion::API::Data::Filters::Tab::Ptr> const& tabs) override
    {
        LOG_SCOPE("");
        using namespace Fluxion::API::Data::Filters;

        auto const active_filters = DummyImpl::ComputeActiveFilters(tabs, GetTableHeader());
        LOG_DEBUG("Active filters size: {}", active_filters.size());

        m_filtered_logs.clear();
        for (auto const& log : m_logs)
        {
            for (auto const& filter : active_filters)
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
    }

    void DisableFilters() override
    {
        m_filtered_logs.clear();
        for (auto const& log : m_logs)
        {
            m_filtered_logs.push_back({.row = log, .metadata = {}});
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

                if (target_row.data.size() < source_row.row.size())
                {
                    target_row.data.resize(source_row.row.size());
                }

                for (std::size_t col_idx = 0; col_idx < source_row.row.size(); ++col_idx)
                {
                    target_row.data[col_idx] = source_row.row[col_idx];
                }

                target_row.metadata = source_row.metadata;
            }
        }
    }

private:
    std::vector<std::vector<std::string>> m_logs;
    std::vector<DummyImpl::FilteredLogRow> m_filtered_logs;
};

} // namespace Fluxion::Application
