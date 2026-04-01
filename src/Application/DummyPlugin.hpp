/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DummyPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief Dummy IFluxionPlugin impl with hardcoded data.
///

#include "Fluxion/API/Data.hpp"
#include "Fluxion/API/IFluxionPlugin.hpp"

#include "Graphite/Logger.hpp"

#include <filesystem>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace Fluxion::Application {

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

    void ApplyFilters(std::vector<Fluxion::API::Data::FiltersTab::Ptr> const& /*tabs*/) override
    {
        // No filtering for dummy plugin
    }

    Fluxion::API::Data::LogsTableHeader GetTableHeader() const override
    {
        return {"Timestamp", "Channel", "Level", "Payload"};
    }

    std::size_t GetTotalLogs() const override { return m_logs.size(); }

    std::size_t GetLogsChunk(
        std::size_t const begin,
        std::size_t const end,
        std::vector<std::vector<std::string>>& out) const override
    {
        LOG_INFO("Begin {} | End {}", begin, end);

        // 1. Validation & Early Exit
        if (m_logs.empty() || begin >= m_logs.size() || begin >= end)
        {
            return 0;
        }

        // 2. Calculate safe bounds
        // We only copy what we actually have in m_logs
        std::size_t const row_count = std::min(end, m_logs.size()) - begin;

        // 3. Perform the copy into the pre-allocated 'out' pool
        for (std::size_t i = 0; i < row_count; ++i)
        {
            auto const& source_row = m_logs[begin + i];
            auto& target_row = out[i];

            GRAPHITE_ASSERT(
                source_row.size() == target_row.size(),
                std::string{"source_row.size() {"} + std::to_string(source_row.size()) +
                    "} does not match target_row.size() {" + std::to_string(target_row.size()) +
                    "}");

            for (std::size_t col_idx = 0; col_idx < target_row.size(); ++col_idx)
            {
                target_row[col_idx] = source_row[col_idx];
            }
        }

        return row_count;
    }

private:
    std::vector<std::vector<std::string>> m_logs;
};

} // namespace Fluxion::Application
