/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file DummyPlugin.hpp
/// @author Alexandru Delegeanu
/// @version 0.2
/// @brief Dummy IFluxionPlugin impl with hardcoded data.
///

#include "Api/Data.hpp"
#include "Api/IFluxionPlugin.hpp"

#include "Core/Logger/Logger.hpp"

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
            Fluxion::API::Data::LogRow entry{};
            entry.push_back(std::string("2026-01-01 12:00:") + (i < 10 ? "0" : "") + std::to_string(i));
            entry.push_back(channels[channel_dist(gen)]);
            entry.push_back(levels[level_dist(gen)]);
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

    void ApplyFilters(Fluxion::API::Data::FilterTabs const& /*tabs*/) override
    {
        // No filtering for dummy plugin
    }

    Fluxion::API::Data::LogsTableHeader GetTableHeader() const override
    {
        return {"Timestamp", "Channel", "Level", "Payload"};
    }

    std::size_t GetTotalLogs() const override { return m_logs.size(); }

    Fluxion::API::Data::LogsChunk GetLogsChunk(std::size_t const begin, std::size_t const end) const override
    {
        Fluxion::API::Data::LogsChunk chunk;
        if (begin >= m_logs.size() || begin >= end)
            return chunk;

        std::size_t actual_end = (end > m_logs.size()) ? m_logs.size() : end;
        chunk.insert(chunk.end(), m_logs.begin() + begin, m_logs.begin() + actual_end);
        return chunk;
    }

private:
    std::vector<Fluxion::API::Data::LogRow> m_logs;
};

} // namespace Fluxion::Application
