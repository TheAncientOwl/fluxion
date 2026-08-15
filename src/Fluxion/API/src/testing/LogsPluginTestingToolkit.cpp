/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsPluginTestingToolkit.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Helper toolkit for testing IFluxionLogsPlugins
///

#include <initializer_list>
#include <print>
#include <random>

#include "Fluxion/API/testing/LogsPluginTestingToolkit.hpp"

namespace Fluxion::API::Testing::LogsPluginTestingKit {

LogsPluginTester::LogsPluginTester(
    std::unique_ptr<ILogsPluginTestWrapper> logs_plugin_test_wrapper,
    TestConfiguration config)
    : m_logs_plugin_test_wrapper{std::move(logs_plugin_test_wrapper)}, m_config{std::move(config)}
{
}

void LogsPluginTester::SetupLogsData()
{
    // Schema matches LogEntry indexing: [0] Timestamp, [1] Channel, [2] Level, [3] Payload
    static constexpr LogEntry::Data header{"Timestamp", "Channel", "Level", "Payload"};
    static constexpr LogEntry::Data channels = {"Channel1", "Channel2", "Channel3", "Channel4"};
    static constexpr LogEntry::Data levels = {"info", "error", "debug", "trace"};

    constexpr std::uint32_t seed = 69420;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> level_dist(0, static_cast<int>(levels.size() - 1));
    std::uniform_int_distribution<> channel_dist(0, static_cast<int>(channels.size() - 1));

    using namespace std::string_literals;

    m_generated_logs.clear();
    m_generated_logs.reserve(m_config.logs_count);

    for (std::size_t log_idx = 1; log_idx <= m_config.logs_count; ++log_idx)
    {
        m_generated_logs.push_back(
            LogEntry::Data{
                std::string("2026-01-01 12:00:") + (log_idx < 10 ? "0" : "") + std::to_string(log_idx),
                channels[static_cast<std::size_t>(channel_dist(gen))],
                levels[static_cast<std::size_t>(level_dist(gen))],
                "Dummy log entry number "s + std::to_string(log_idx) + " ---------------------------"});

        m_logs_plugin_test_wrapper->WriteLogEntryToImportFile(header, m_generated_logs.back());
    }
    m_logs_plugin_test_wrapper->OnLogsGenerationProcessDone();
}

void LogsPluginTester::TeardownLogsData()
{
    m_generated_logs.clear();
}

TestResults& TestResults::operator+=(TestResult const& res)
{
    ++total;
    passed += (res.state == ETestState::Passed ? 1 : 0);
    return *this;
}

TestResults LogsPluginTester::RunTests()
{
    auto results{TestResults{}};

    auto const execute_test = [&](auto&& test_fn, std::string_view test_name) {
        RunTest([&]() {
            auto const res = test_fn();
            results += res;
            if (res.state != ETestState::Passed)
            {
                std::println("[FAIL] {}: {}", test_name, res.message);
            }
        });
    };

    execute_test([this]() { return TestIO(); }, "TestIO");
    execute_test([this]() { return TestOutOfBoundsQuery(); }, "TestOutOfBoundsQuery");
    execute_test(
        [this]() { return TestEmptyAndOverlappingRanges(); }, "TestEmptyAndOverlappingRanges");
    execute_test([this]() { return TestMetadataAndHeader(); }, "TestMetadataAndHeader");
    execute_test([this]() { return TestNavigationAPI(); }, "TestNavigationAPI");
    execute_test([this]() { return TestFilterLifecycle(); }, "TestFilterLifecycle");
    execute_test([this]() { return TestEnableDisableLifecycle(); }, "TestEnableDisableLifecycle");

    return results;
}

TestResult LogsPluginTester::TestIO()
{
    using namespace std::string_literals;
    auto const& logs_plugin{m_logs_plugin_test_wrapper->GetLogsPlugin()};

    if (logs_plugin.GetTotalLogs() != m_generated_logs.size())
    {
        return {
            ETestState::Failed,
            "Total log count mismatch: expected "s + std::to_string(m_generated_logs.size()) +
                ", got " + std::to_string(logs_plugin.GetTotalLogs())};
    }

    static std::vector<Fluxion::API::LogsPlugin::Data::Range> const ranges{{0, 2}, {2, 5}};

    Fluxion::API::LogsPlugin::Data::IndexToLogRowMap index_to_log_row_map{};
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter writer{index_to_log_row_map};

    logs_plugin.GetLogs(ranges, writer);

    static std::initializer_list<std::size_t> const target_indices{0u, 1u, 2u, 3u, 4u};

    for (auto const index : target_indices)
    {
        if (!index_to_log_row_map.contains(index))
        {
            return {ETestState::Failed, "Missing imported index: "s + std::to_string(index)};
        }

        auto const& plugin_log = index_to_log_row_map.at(index);
        auto const& generated_log = m_generated_logs[index];

        if (plugin_log.data.size() != generated_log.size())
        {
            return {
                ETestState::Failed,
                "Log line size mismatch at index "s + std::to_string(index) +
                    ": plugin == " + std::to_string(plugin_log.data.size()) +
                    " | generated == " + std::to_string(generated_log.size())};
        }

        for (std::size_t field_idx = 0; field_idx < generated_log.size(); ++field_idx)
        {
            if (plugin_log.data[field_idx] != generated_log[field_idx])
            {
                return {
                    ETestState::Failed,
                    "Field value mismatch at index "s + std::to_string(index) + " [field " +
                        std::to_string(field_idx) + "]: plugin == \"" + plugin_log.data[field_idx] +
                        "\" | generated == \"" + generated_log[field_idx] + "\""};
            }
        }
    }

    return {ETestState::Passed, "OK"};
}

TestResult LogsPluginTester::TestOutOfBoundsQuery()
{
    using namespace std::string_literals;
    auto const& logs_plugin{m_logs_plugin_test_wrapper->GetLogsPlugin()};

    const std::size_t total_logs = logs_plugin.GetTotalLogs();
    std::vector<Fluxion::API::LogsPlugin::Data::Range> const ranges{{total_logs + 10, total_logs + 20}};

    Fluxion::API::LogsPlugin::Data::IndexToLogRowMap index_to_log_row_map{};
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter writer{index_to_log_row_map};

    logs_plugin.GetLogs(ranges, writer);

    if (!index_to_log_row_map.empty())
    {
        return {
            ETestState::Failed,
            "Out-of-bounds query populated entries unexpectedly. Returned count: "s +
                std::to_string(index_to_log_row_map.size())};
    }

    return {ETestState::Passed, "OK"};
}

TestResult LogsPluginTester::TestEmptyAndOverlappingRanges()
{
    using namespace std::string_literals;
    auto const& logs_plugin{m_logs_plugin_test_wrapper->GetLogsPlugin()};

    // Overlapping ranges: [1, 4] and [3, 5] -> expected indices: 1, 2, 3, 4
    std::vector<Fluxion::API::LogsPlugin::Data::Range> const ranges{{1, 4}, {3, 5}, {10, 10}};

    Fluxion::API::LogsPlugin::Data::IndexToLogRowMap index_to_log_row_map{};
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMapWriter writer{index_to_log_row_map};

    logs_plugin.GetLogs(ranges, writer);

    static std::initializer_list<std::size_t> const expected_indices{1u, 2u, 3u, 4u};

    for (auto const index : expected_indices)
    {
        if (!index_to_log_row_map.contains(index))
        {
            return {
                ETestState::Failed,
                "Overlapping query missed expected index: "s + std::to_string(index)};
        }
    }

    if (index_to_log_row_map.contains(10))
    {
        return {ETestState::Failed, "Zero-length range {10, 10} should not introduce index 10"};
    }

    return {ETestState::Passed, "OK"};
}

TestResult LogsPluginTester::TestMetadataAndHeader()
{
    using namespace std::string_literals;
    auto const& logs_plugin{m_logs_plugin_test_wrapper->GetLogsPlugin()};

    if (logs_plugin.GetDisplayName().empty())
    {
        return {ETestState::Failed, "GetDisplayName() returned an empty string_view"};
    }

    auto const table_header = logs_plugin.GetTableHeader();
    if (table_header.empty())
    {
        return {ETestState::Failed, "GetTableHeader() returned empty column details"};
    }

    if (table_header.size() != m_generated_logs.front().size())
    {
        return {
            ETestState::Failed,
            "Header column count mismatch: expected "s +
                std::to_string(m_generated_logs.front().size()) + ", got " +
                std::to_string(table_header.size())};
    }

    return {ETestState::Passed, "OK"};
}

TestResult LogsPluginTester::TestNavigationAPI()
{
    using namespace std::string_literals;
    auto& logs_plugin{m_logs_plugin_test_wrapper->GetLogsPlugin()};

    Graphite::Common::Utility::UniqueID const empty_filter_id{};

    // Forward navigation test
    auto next_index = logs_plugin.GetNextLog(empty_filter_id, 0);
    if (!next_index.has_value())
    {
        return {ETestState::Failed, "GetNextLog failed to find next entry from index 0"};
    }

    if (*next_index <= 0)
    {
        return {
            ETestState::Failed,
            "GetNextLog from index 0 returned non-forward index: "s + std::to_string(*next_index)};
    }

    // Edge case: Beyond bounds forward navigation
    const std::size_t last_index = logs_plugin.GetTotalLogs();
    auto out_of_bounds_next = logs_plugin.GetNextLog(empty_filter_id, last_index);
    if (out_of_bounds_next != 0)
    {
        return {
            ETestState::Failed,
            "GetNextLog failed to circle back: "s + std::to_string(*out_of_bounds_next)};
    }

    // Backward navigation test
    auto prev_index = logs_plugin.GetPrevLog(empty_filter_id, last_index);
    if (!prev_index.has_value())
    {
        return {ETestState::Failed, "GetPrevLog failed to find previous entry from last index"};
    }

    if (*prev_index >= last_index)
    {
        return {
            ETestState::Failed,
            "GetPrevLog from last index returned non-backward index: "s + std::to_string(*prev_index)};
    }

    // Edge case: Backward navigation from index 0
    auto out_of_bounds_prev = logs_plugin.GetPrevLog(empty_filter_id, 0);
    if (out_of_bounds_prev != logs_plugin.GetTotalLogs() - 1)
    {
        return {
            ETestState::Failed,
            "GetPrevLog from index 0 failed to circle back: "s + std::to_string(*out_of_bounds_prev)};
    }

    return {ETestState::Passed, "OK"};
}

TestResult LogsPluginTester::TestFilterLifecycle()
{
    using namespace std::string_literals;
    auto& logs_plugin{m_logs_plugin_test_wrapper->GetLogsPlugin()};

    const std::size_t initial_total = logs_plugin.GetTotalLogs();

    // Applying empty filters should maintain state without crashing
    logs_plugin.ApplyFilters({}, {});

    if (logs_plugin.GetTotalLogs() > initial_total)
    {
        return {
            ETestState::Failed,
            "GetTotalLogs increased unexpectedly after applying empty filters: "s +
                std::to_string(logs_plugin.GetTotalLogs())};
    }

    logs_plugin.DisableFilters();

    if (logs_plugin.GetTotalLogs() != initial_total)
    {
        return {
            ETestState::Failed,
            "GetTotalLogs after DisableFilters() mismatch: expected "s +
                std::to_string(initial_total) + ", got " + std::to_string(logs_plugin.GetTotalLogs())};
    }

    return {ETestState::Passed, "OK"};
}

TestResult LogsPluginTester::TestEnableDisableLifecycle()
{
    auto& logs_plugin{m_logs_plugin_test_wrapper->GetLogsPlugin()};

    // Verify calling plugin lifecycle hooks does not cause crashes or state corruption
    Fluxion::API::LogsPlugin::Data::OnEnableData enable_data{};
    logs_plugin.OnEnable(enable_data);

    Fluxion::API::LogsPlugin::Data::OnDisableData disable_data{};
    logs_plugin.OnDisable(disable_data);

    return {ETestState::Passed, "OK"};
}

} // namespace Fluxion::API::Testing::LogsPluginTestingKit