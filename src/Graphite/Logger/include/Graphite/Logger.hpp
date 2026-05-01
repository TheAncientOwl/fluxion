/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Logger.hpp
/// @author Alexandru Delegeanu
/// @version 1.14
/// @brief Logging utilities
///

#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream> // IWYU pragma: keep
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#include "imgui.h"

#include "Graphite/Common/Utility/TWithFlags.hpp"
#include "Graphite/Settings/SettingsManager.hpp"

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef GRAPHITE_LOGGER_BUILD
#define GRAPHITE_LOGGER_API __declspec(dllexport)
#else
#define GRAPHITE_LOGGER_API __declspec(dllimport)
#endif
#else
#ifdef GRAPHITE_LOGGER_BUILD
#define GRAPHITE_LOGGER_API __attribute__((visibility("default")))
#else
#define GRAPHITE_LOGGER_API
#endif
#endif

namespace Graphite::Logger {

class Logger;
GRAPHITE_LOGGER_API Logger& GetLogger();

// clang-format off
enum class ELogLevel : std::uint8_t
{
    Scope    = 1 << 0, // 00000001
    Info     = 1 << 1, // 00000010
    Warn     = 1 << 2, // 00000100
    Error    = 1 << 3, // 00001000
    Critical = 1 << 4, // 00010000
    Debug    = 1 << 5, // 00100000
    Trace    = 1 << 6, // 01000000
};
// clang-format on

struct LogMessage
{
    ELogLevel level;
    std::string scope;
    std::string message;
    std::chrono::system_clock::time_point time;
};

struct LogScopeFlags : public Graphite::Common::Utility::TWithFlags<LogScopeFlags, ELogLevel>
{
    using Base = Graphite::Common::Utility::TWithFlags<LogScopeFlags, ELogLevel>;
    using Storage = Base::Storage;

    [[nodiscard]] Storage GetStorage() const noexcept { return this->flags; }

    void SetStorage(Storage storage) noexcept { this->flags = storage; }
};

struct GlobalLogLevel
{
    GlobalLogLevel(ELogLevel const value, std::string icon, std::string label, ImVec4 const color);
    ELogLevel value{};
    std::string icon{};
    std::string label{};
    std::string display{};
    ImVec4 color{};
};

struct StringHash
{
    using is_transparent = void;

    [[nodiscard]] size_t operator()(std::string_view sv) const
    {
        return std::hash<std::string_view>{}(sv);
    }

    [[nodiscard]] size_t operator()(std::string const& str) const
    {
        return std::hash<std::string>{}(str);
    }
};

class Logger
{
public: // Types
    using ScopeEnabledMap =
        std::unordered_map<std::string, LogScopeFlags, StringHash, std::equal_to<>>;
    using LogLevels = std::array<GlobalLogLevel, 7>;

public: // API
    Logger();
    ~Logger();

    std::string_view DefineLogScope(std::string_view scope);

    template <typename... Args>
    void Log(ELogLevel level, std::string_view scope, std::format_string<Args...> fmt, Args&&... args)
    {
        if (!IsLevelEnabled(level))
        {
            return;
        }

        {
            std::lock_guard lock{m_scope_mutex};
            auto it = m_scope_enabled.find(scope);

            if (it == m_scope_enabled.end())
            {
                it = m_scope_enabled.emplace(std::string{scope}, GetDefaultScopeFlags()).first;
            }

            if (!it->second[level])
            {
                return;
            }
        }

        Enqueue(
            LogMessage{
                level,
                std::string(scope),
                std::format(fmt, std::forward<Args>(args)...),
                std::chrono::system_clock::now()});
    }

    void SaveConfig();
    void LoadConfig();

    ScopeEnabledMap const& GetScopes() const;
    static LogLevels const& GetLevels();

    void SetScopeEnabled(std::string_view scope, bool const enabled);
    void SetScopeLevelEnabled(std::string_view scope, ELogLevel const level, bool const enabled);

    void SetLevelState(ELogLevel const level, bool const enabled);
    [[nodiscard]] bool IsLevelEnabled(ELogLevel const level) const noexcept;

private:
    static std::filesystem::path GetConfigFilePath();
    static std::filesystem::path GetLogFilePath();

    void Enqueue(LogMessage&& msg);
    void ProcessQueue();
    void PrintMessage(LogMessage const& msg);

    static LogScopeFlags GetDefaultScopeFlags();
    static std::string const& GetGlobalScopeKey();

private:
    std::atomic<bool> m_running;
    std::ofstream m_log_file;
    std::thread m_worker;
    std::condition_variable m_cv;

    std::mutex m_queue_mutex;
    std::queue<LogMessage> m_queue;

    std::mutex m_scope_mutex;
    ScopeEnabledMap m_scope_enabled;
    std::atomic<uint8_t> m_global_level_mask;

    std::vector<std::string> m_queued_defined_scopes{};
    Graphite::Settings::SettingsManager m_settings_manager;
};

class ScopeLogger
{
public:
    // Scope can be string_view because __PRETTY_FUNCTION__ has static lifetime.
    // Tag MUST be std::string because std::format returns a temporary that would dangle.
    ScopeLogger(std::string tag, std::string_view scope);
    ~ScopeLogger();

private:
    std::string_view m_scope;
    std::string m_tag;
    std::chrono::high_resolution_clock::time_point m_start;
};

} // namespace Graphite::Logger

#ifdef GRAPHITE_NO_LOGGER
#define DEFINE_LOG_SCOPE(name)
#define USE_LOG_SCOPE(scope)
#define LOG_TRACE(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_CRITICAL(fmt, ...)
#define LOG_DEBUG(fmt, ...)
#define LOG_SCOPE(fmt, ...)
#define GRAPHITE_ASSERT(condition, message)
#else
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define DEFINE_LOG_SCOPE(name)                                                \
    namespace {                                                               \
    [[maybe_unused]] static auto const CONCAT(                                \
        CONCAT(__defined_graphite_log_scope_, __LINE__),                      \
        __COUNTER__) = ::Graphite::Logger::GetLogger().DefineLogScope(#name); \
    }

#define USE_LOG_SCOPE(scope) auto const __graphite_log_scope = #scope

#define LOG_TRACE(fmt, ...)              \
    ::Graphite::Logger::GetLogger().Log( \
        ::Graphite::Logger::ELogLevel::Trace, __graphite_log_scope, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_INFO(fmt, ...)               \
    ::Graphite::Logger::GetLogger().Log( \
        ::Graphite::Logger::ELogLevel::Info, __graphite_log_scope, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_WARN(fmt, ...)               \
    ::Graphite::Logger::GetLogger().Log( \
        ::Graphite::Logger::ELogLevel::Warn, __graphite_log_scope, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_ERROR(fmt, ...)              \
    ::Graphite::Logger::GetLogger().Log( \
        ::Graphite::Logger::ELogLevel::Error, __graphite_log_scope, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_CRITICAL(fmt, ...)                                                                          \
    ::Graphite::Logger::GetLogger().Log(                                                                \
        ::Graphite::Logger::ELogLevel::Critical, __graphite_log_scope, fmt __VA_OPT__(, ) __VA_ARGS__); \
    std::this_thread::sleep_for(std::chrono::seconds{2})

#define LOG_DEBUG(fmt, ...)              \
    ::Graphite::Logger::GetLogger().Log( \
        ::Graphite::Logger::ELogLevel::Debug, __graphite_log_scope, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_SCOPE(fmt, ...)                                               \
    ::Graphite::Logger::ScopeLogger _graphite_scope_logger                \
    {                                                                     \
        std::format(fmt __VA_OPT__(, ) __VA_ARGS__), __graphite_log_scope \
    }

#define GRAPHITE_ASSERT(condition, message)                                                  \
    do                                                                                       \
    {                                                                                        \
        if (!(condition))                                                                    \
        {                                                                                    \
            LOG_CRITICAL(                                                                    \
                "Assertion failed {} on line {}: {} | Condition: {}",                        \
                __FILE__,                                                                    \
                __LINE__,                                                                    \
                message,                                                                     \
                #condition);                                                                 \
            std::cerr << std::endl                                                           \
                      << "Assertion failed " << __FILE__ << " on line " << __LINE__ << " : " \
                      << message << " | Condition: " << #condition << std::endl;             \
            std::abort();                                                                    \
        }                                                                                    \
    } while (0)
#endif
