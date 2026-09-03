/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsPluginTestingToolkit.hpp
/// @author Alexandru Delegeanu
/// @version 1.1
/// @brief Helper toolkit for testing IFluxionLogsPlugins
///

#pragma once

#include <array>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"
#include "Graphite/Logger.hpp"

namespace Fluxion::API::Testing::LogsPluginTestingKit {

namespace LogEntry {

using Data = std::array<std::string, 4>;

inline constexpr std::string_view GetTimestamp(Data const& data) noexcept
{
    return data[0];
}
inline constexpr std::string_view GetChannel(Data const& data) noexcept
{
    return data[1];
}
inline constexpr std::string_view GetLevel(Data const& data) noexcept
{
    return data[2];
}
inline constexpr std::string_view GetPayload(Data const& data) noexcept
{
    return data[3];
}

} // namespace LogEntry

class ILogsPluginTestWrapper
{
public:
    virtual ~ILogsPluginTestWrapper() = default;
    ILogsPluginTestWrapper() = default;

    virtual void Setup(std::filesystem::path const& plugin_home_path) = 0;
    virtual void Teardown() = 0;
    virtual void WriteLogEntryToImportFile(LogEntry::Data const& header, LogEntry::Data const& entry) = 0;
    virtual void OnLogsGenerationProcessDone() = 0;
    virtual Fluxion::API::LogsPlugin::IFluxionLogsPlugin& GetLogsPlugin() = 0;
    virtual std::filesystem::path GetImportLogsFilePath() const = 0;
};

struct TestConfiguration
{
    std::size_t logs_count{128};
    std::uint32_t seed{69420};
};

template <typename TWrapper>
class LogsPluginTestFixture : public ::testing::Test
{
protected:
    explicit LogsPluginTestFixture(TestConfiguration const& config = TestConfiguration{})
        : m_config(config)
    {
    }

    void SetUp() override
    {
        Graphite::Logger::DisableAllScopes();

        m_tmp_path = std::filesystem::temp_directory_path() / "fluxion_test_tmp";
        std::filesystem::create_directories(m_tmp_path);

        m_wrapper = std::make_unique<TWrapper>();
        m_wrapper->Setup(m_tmp_path);
        SetupLogsData();

        m_wrapper->GetLogsPlugin().ImportLogs(m_wrapper->GetImportLogsFilePath());
    }

    void TearDown() override
    {
        if (m_wrapper)
        {
            m_wrapper->Teardown();
            m_wrapper.reset();
        }
        TeardownLogsData();

        if (std::filesystem::exists(m_tmp_path))
        {
            std::filesystem::remove_all(m_tmp_path);
        }
    }

    void SetupLogsData()
    {
        static constexpr LogEntry::Data header{"Timestamp", "Channel", "Level", "Payload"};
        static constexpr std::array<std::string_view, 4> channels = {
            "Channel1", "Channel2", "Channel3", "Channel4"};
        static constexpr std::array<std::string_view, 4> levels = {
            "info", "error", "debug", "trace"};

        std::mt19937 gen(m_config.seed);
        std::uniform_int_distribution<> level_dist(0, static_cast<int>(levels.size() - 1));
        std::uniform_int_distribution<> channel_dist(0, static_cast<int>(channels.size() - 1));

        using namespace std::string_literals;

        m_generated_logs.clear();
        m_generated_logs.reserve(m_config.logs_count);

        for (std::size_t log_idx = 1; log_idx <= m_config.logs_count; ++log_idx)
        {
            m_generated_logs.push_back(
                LogEntry::Data{
                    std::string("2026-01-01 12:00:") + (log_idx < 10 ? "0" : "") +
                        std::to_string(log_idx),
                    std::string(channels[static_cast<std::size_t>(channel_dist(gen))]),
                    std::string(levels[static_cast<std::size_t>(level_dist(gen))]),
                    "Dummy log entry number "s + std::to_string(log_idx) +
                        " ---------------------------"});

            m_wrapper->WriteLogEntryToImportFile(header, m_generated_logs.back());
        }
        m_wrapper->OnLogsGenerationProcessDone();
    }

    void TeardownLogsData() { m_generated_logs.clear(); }

    // --- Reusable Test Implementations ---

    void RunTestIO()
    {
        auto& logs_plugin = m_wrapper->GetLogsPlugin();
        EXPECT_EQ(logs_plugin.GetTotalLogs(), m_generated_logs.size());

        static std::vector<Fluxion::API::LogsPlugin::Data::Range> const ranges{{0, 2}, {2, 5}};
        Fluxion::API::LogsPlugin::Data::IndexToLogRowMap index_to_log_row_map{};
        Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter writer{index_to_log_row_map};

        logs_plugin.GetLogs(ranges, writer);

        static std::initializer_list<std::size_t> const target_indices{0u, 1u, 2u, 3u, 4u};
        for (auto const index : target_indices)
        {
            ASSERT_TRUE(index_to_log_row_map.contains(index)) << "Missing imported index: " << index;
            auto const& plugin_log = index_to_log_row_map.at(index);
            auto const& generated_log = m_generated_logs[index];

            EXPECT_EQ(plugin_log.data.size(), generated_log.size());
            for (std::size_t field_idx = 0; field_idx < generated_log.size(); ++field_idx)
            {
                EXPECT_EQ(plugin_log.data[field_idx], generated_log[field_idx]);
            }
        }
    }

    void RunTestReadAllLogs()
    {
        auto& logs_plugin = m_wrapper->GetLogsPlugin();
        const std::size_t total_logs = logs_plugin.GetTotalLogs();
        EXPECT_EQ(total_logs, m_generated_logs.size());

        if (total_logs == 0)
        {
            return;
        }

        std::vector<Fluxion::API::LogsPlugin::Data::Range> const ranges{{0, total_logs}};

        Fluxion::API::LogsPlugin::Data::IndexToLogRowMap index_to_log_row_map{};
        Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter writer{index_to_log_row_map};

        logs_plugin.GetLogs(ranges, writer);

        for (std::size_t index = 0; index < total_logs; ++index)
        {
            ASSERT_TRUE(index_to_log_row_map.contains(index)) << "Missing log at index: " << index;
            auto const& plugin_log = index_to_log_row_map.at(index);
            auto const& generated_log = m_generated_logs[index];

            EXPECT_EQ(plugin_log.data.size(), generated_log.size())
                << "Size mismatch at index: " << index;
            for (std::size_t field_idx = 0; field_idx < generated_log.size(); ++field_idx)
            {
                EXPECT_EQ(plugin_log.data[field_idx], generated_log[field_idx])
                    << "Mismatch at index " << index << ", field " << field_idx;
            }
        }
    }

    void RunTestOutOfBoundsQuery()
    {
        auto& logs_plugin = m_wrapper->GetLogsPlugin();
        const std::size_t total_logs = logs_plugin.GetTotalLogs();
        std::vector<Fluxion::API::LogsPlugin::Data::Range> const ranges{
            {total_logs + 10, total_logs + 20}};

        Fluxion::API::LogsPlugin::Data::IndexToLogRowMap index_to_log_row_map{};
        Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter writer{index_to_log_row_map};

        logs_plugin.GetLogs(ranges, writer);
        EXPECT_TRUE(index_to_log_row_map.empty());
    }

    void RunTestEmptyAndOverlappingRanges()
    {
        auto& logs_plugin = m_wrapper->GetLogsPlugin();
        std::vector<Fluxion::API::LogsPlugin::Data::Range> const ranges{{1, 4}, {3, 5}, {10, 10}};

        Fluxion::API::LogsPlugin::Data::IndexToLogRowMap index_to_log_row_map{};
        Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter writer{index_to_log_row_map};

        logs_plugin.GetLogs(ranges, writer);

        static std::initializer_list<std::size_t> const expected_indices{1u, 2u, 3u, 4u};
        for (auto const index : expected_indices)
        {
            EXPECT_TRUE(index_to_log_row_map.contains(index));
        }
        EXPECT_FALSE(index_to_log_row_map.contains(10));
    }

    void RunTestMetadataAndHeader()
    {
        auto& logs_plugin = m_wrapper->GetLogsPlugin();
        EXPECT_FALSE(logs_plugin.GetDisplayName().empty());
        EXPECT_FALSE(logs_plugin.GetDirectoryName().empty());

        auto const table_header = logs_plugin.GetTableHeader();
        EXPECT_FALSE(table_header.empty());
        EXPECT_EQ(table_header.size(), m_generated_logs.front().size());
    }

    void RunTestNavigationAPI()
    {
        auto& logs_plugin = m_wrapper->GetLogsPlugin();
        Graphite::Common::Utility::UniqueID const filter_id{
            Graphite::Common::Utility::UniqueID::GetDefault()};

        auto next_index = logs_plugin.GetNextLog(filter_id, 0);
        ASSERT_TRUE(next_index.has_value());
        EXPECT_GT(*next_index, 0);

        const std::size_t last_index = logs_plugin.GetTotalLogs();
        auto out_of_bounds_next = logs_plugin.GetNextLog(filter_id, last_index);
        EXPECT_EQ(out_of_bounds_next, 0);

        auto prev_index = logs_plugin.GetPrevLog(filter_id, last_index);
        ASSERT_TRUE(prev_index.has_value());
        EXPECT_LT(*prev_index, last_index);

        auto out_of_bounds_prev = logs_plugin.GetPrevLog(filter_id, 0);
        EXPECT_EQ(out_of_bounds_prev, logs_plugin.GetTotalLogs() - 1);
    }

    void RunTestFilterLifecycle()
    {
        auto& logs_plugin = m_wrapper->GetLogsPlugin();
        const std::size_t initial_total = logs_plugin.GetTotalLogs();

        logs_plugin.ApplyFilters({}, {});
        EXPECT_LE(logs_plugin.GetTotalLogs(), initial_total);

        logs_plugin.DisableFilters();
        EXPECT_EQ(logs_plugin.GetTotalLogs(), initial_total);
    }

    void RunTestEnableDisableLifecycle()
    {
        auto& logs_plugin = m_wrapper->GetLogsPlugin();
        Fluxion::API::LogsPlugin::Data::OnEnableData enable_data{};
        logs_plugin.OnEnable(enable_data);

        Fluxion::API::LogsPlugin::Data::OnDisableData disable_data{};
        logs_plugin.OnDisable(disable_data);
    }

    std::filesystem::path m_tmp_path{};
    std::unique_ptr<ILogsPluginTestWrapper> m_wrapper{};
    TestConfiguration m_config{};
    std::vector<LogEntry::Data> m_generated_logs{};
};

} // namespace Fluxion::API::Testing::LogsPluginTestingKit

/// Macro accepting a whole TestConfiguration struct via variadic arguments
#define FLUXION_DEFINE_LOGS_PLUGIN_TESTS(TWrapper, ...)                                         \
    class LogsPluginTest_##TWrapper                                                             \
        : public ::Fluxion::API::Testing::LogsPluginTestingKit::LogsPluginTestFixture<TWrapper> \
    {                                                                                           \
    public:                                                                                     \
        LogsPluginTest_##TWrapper()                                                             \
            : ::Fluxion::API::Testing::LogsPluginTestingKit::LogsPluginTestFixture<TWrapper>(   \
                  ::Fluxion::API::Testing::LogsPluginTestingKit::TestConfiguration __VA_ARGS__) \
        {                                                                                       \
        }                                                                                       \
    };                                                                                          \
                                                                                                \
    TEST_F(LogsPluginTest_##TWrapper, TestIO)                                                   \
    {                                                                                           \
        this->RunTestIO();                                                                      \
    }                                                                                           \
    TEST_F(LogsPluginTest_##TWrapper, TestReadAllLogs)                                          \
    {                                                                                           \
        this->RunTestReadAllLogs();                                                             \
    }                                                                                           \
    TEST_F(LogsPluginTest_##TWrapper, TestOutOfBoundsQuery)                                     \
    {                                                                                           \
        this->RunTestOutOfBoundsQuery();                                                        \
    }                                                                                           \
    TEST_F(LogsPluginTest_##TWrapper, TestEmptyAndOverlappingRanges)                            \
    {                                                                                           \
        this->RunTestEmptyAndOverlappingRanges();                                               \
    }                                                                                           \
    TEST_F(LogsPluginTest_##TWrapper, TestMetadataAndHeader)                                    \
    {                                                                                           \
        this->RunTestMetadataAndHeader();                                                       \
    }                                                                                           \
    TEST_F(LogsPluginTest_##TWrapper, TestNavigationAPI)                                        \
    {                                                                                           \
        this->RunTestNavigationAPI();                                                           \
    }                                                                                           \
    TEST_F(LogsPluginTest_##TWrapper, TestFilterLifecycle)                                      \
    {                                                                                           \
        this->RunTestFilterLifecycle();                                                         \
    }                                                                                           \
    TEST_F(LogsPluginTest_##TWrapper, TestEnableDisableLifecycle)                               \
    {                                                                                           \
        this->RunTestEnableDisableLifecycle();                                                  \
    }
