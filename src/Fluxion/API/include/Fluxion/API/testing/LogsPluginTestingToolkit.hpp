/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsPluginTestingToolkit.hpp
/// @author Alexandru Delegeanu
/// @version 0.4
/// @brief Helper toolkit for testing IFluxionLogsPlugins
///

#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"

namespace Fluxion::API::Testing::LogsPluginTestingKit {

namespace LogEntry {

// | timestamp | channel | level | payload |
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

    /**
     * @brief Create logs plugin and setup its internals
     */
    virtual void Setup(std::filesystem::path const& plugin_home_path) = 0;

    /**
     * @brief Cleanup
     */
    virtual void Teardown() = 0;

    /**
     * @brief Used to generate input data file during @see LogsPluginTestSuite::Setup
     *
     * @param header
     * @param entry
     */
    virtual void WriteLogEntryToImportFile(LogEntry::Data const& header, LogEntry::Data const& entry) = 0;

    /**
     * @brief Cleanup after @see LogsPluginTestSuite::WriteLogEntryToImportFile
     *
     * @param header
     * @param entry
     */
    virtual void OnLogsGenerationProcessDone() = 0;

    /**
     * @brief Get the Logs Plugin object
     *
     * @return Fluxion::API::LogsPlugin::IFluxionLogsPlugin&
     */
    virtual Fluxion::API::LogsPlugin::IFluxionLogsPlugin& GetLogsPlugin() = 0;

    /**
     * @brief Get the path to the raw format log file ~ .txt / .csv / .whatever ~ for import
     *
     * @return std::filesystem::path
     */
    virtual std::filesystem::path GetImportLogsFilePath() const = 0;
};

struct TestConfiguration
{
    std::size_t logs_count{128};
};

enum class ETestState
{
    Init = 0,
    Passed = 1,
    Failed = 2
};

struct TestResult
{
    ETestState state{ETestState::Init};
    std::string message{};
    std::string name{"not-set"};

    TestResult& setName(std::string_view const test_name);
};

class LogsPluginTester
{
public:
    LogsPluginTester(
        std::unique_ptr<ILogsPluginTestWrapper> logs_plugin_test_wrapper,
        TestConfiguration config);

public:
    void RunTests();
    void PrintTestResults() const;
    bool AllPassed() const;

private: // Testing API
    template <typename TestCallable>
    void RunTest(TestCallable&& test_callable)
    {
        auto const tmp_path{std::filesystem::temp_directory_path() / "fluxion_test_tmp"};
        std::filesystem::create_directories(tmp_path);

        m_logs_plugin_test_wrapper->Setup(tmp_path);
        SetupLogsData();

        m_logs_plugin_test_wrapper->GetLogsPlugin().ImportLogs(
            m_logs_plugin_test_wrapper->GetImportLogsFilePath());

        m_results.push_back(test_callable());

        m_logs_plugin_test_wrapper->Teardown();
        TeardownLogsData();

        if (std::filesystem::exists(tmp_path))
        {
            std::filesystem::remove_all(tmp_path);
        }
    }

    void SetupLogsData();
    void TeardownLogsData();

private: // Tests
    TestResult TestIO();
    TestResult TestOutOfBoundsQuery();
    TestResult TestEmptyAndOverlappingRanges();
    TestResult TestMetadataAndHeader();
    TestResult TestNavigationAPI();
    TestResult TestFilterLifecycle();
    TestResult TestEnableDisableLifecycle();

private:
    std::unique_ptr<ILogsPluginTestWrapper> m_logs_plugin_test_wrapper{};
    TestConfiguration m_config{};
    std::vector<LogEntry::Data> m_generated_logs{};
    std::vector<TestResult> m_results{};
};

} // namespace Fluxion::API::Testing::LogsPluginTestingKit