/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Logger.hpp
/// @author Alexandru Delegeanu
/// @version 1.7
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
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

#include "Core/Common/TWithFlags.hpp"

namespace Graphite::Core::Logger {

// clang-format off
enum class ELogLevel : std::uint8_t
{
    Trace    = 1 << 0, // 00000001
    Info     = 1 << 1, // 00000010
    Warn     = 1 << 2, // 00000100
    Error    = 1 << 3, // 00001000
    Critical = 1 << 4, // 00010000
    Debug    = 1 << 5, // 00100000
    Scope    = 1 << 6, // 01000000
};
// clang-format on

struct LogMessage
{
    ELogLevel level;
    std::string scope;
    std::string message;
    std::chrono::system_clock::time_point time;
};

struct LogScopeFlags : public Graphite::Common::TWithFlags<LogScopeFlags, ELogLevel>
{
    using Base = Graphite::Common::TWithFlags<LogScopeFlags, ELogLevel>;
    using Storage = Base::Storage;

    [[nodiscard]]
    Storage GetStorage() const noexcept
    {
        return this->flags;
    }

    void SetStorage(Storage storage) noexcept { this->flags = storage; }
};

class Logger
{
public:
    ~Logger();

    template <typename... Args>
    static void log(ELogLevel level, std::string scope, std::format_string<Args...> fmt, Args&&... args)
    {
        {
            std::string key(scope);
            std::lock_guard lock(s_scope_mutex);

            auto it = s_scope_enabled.find(key);
            if (it == s_scope_enabled.end())
            {
                auto flags = GetDefaultScopeFlags();
                s_scope_enabled.emplace(key, flags);
            }
        }

        if (!IsLevelEnabled(level))
        {
            return;
        }

        {
            std::string key(scope);
            std::lock_guard lock(s_scope_mutex);

            auto it = s_scope_enabled.find(key);
            if (it != s_scope_enabled.end())
            {
                if (!it->second[level])
                    return;
            }
        }

        Instance().Enqueue(
            LogMessage{
                level,
                scope,
                std::format(fmt, std::forward<Args>(args)...),
                std::chrono::system_clock::now()});
    }

public:
    static void SaveConfig();
    static void LoadConfig();
    static std::filesystem::path GetConfigFilePath();

private:
    static std::filesystem::path GetLogFilePath();
    static Logger& Instance();
    Logger();

    void Enqueue(LogMessage&& msg);
    void ProcessQueue();
    void PrintMessage(LogMessage const& msg);
    static LogScopeFlags GetDefaultScopeFlags();
    static std::string const& GetGlobalScopeKey();

public:
    using ScopeEnabledMap = std::unordered_map<std::string, LogScopeFlags>;
    static ScopeEnabledMap GetScopes();
    static void SetScopeEnabled(std::string scope, bool enabled);
    static void SetScopeLevelState(std::string scope, ELogLevel const level, bool const enabled);

    struct LogLevel
    {
        LogLevel(ELogLevel const value, std::string icon, std::string label);
        ELogLevel value{};
        std::string icon{};
        std::string label{};
        std::string display{};
    };
    using LogLevels = std::array<LogLevel, 7>;
    static LogLevels const& GetLevels();
    static void SetLevelState(ELogLevel const level, bool const enabled);
    static bool IsLevelEnabled(ELogLevel const level);

private:
    std::queue<LogMessage> m_queue;
    std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_running;
    std::thread m_worker;
    std::ofstream m_log_file;

    static std::mutex s_scope_mutex;
    static ScopeEnabledMap s_scope_enabled;
};

class ScopeLogger
{
public:
    ScopeLogger(std::string tag, std::string scope);
    ~ScopeLogger();

private:
    std::string m_scope;
    std::string m_tag;
    std::chrono::high_resolution_clock::time_point m_start;
};

} // namespace Graphite::Core::Logger

#ifdef GRAPHITE_NO_LOGGER
#define LOG_TRACE(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_CRITICAL(fmt, ...)
#define LOG_DEBUG(fmt, ...)
#define LOG_SCOPE(fmt, ...)
#define GRAPHITE_ASSERT(condition, message)
#else
#define LOG_TRACE(fmt, ...)                \
    ::Graphite::Core::Logger::Logger::log( \
        ::Graphite::Core::Logger::ELogLevel::Trace, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_INFO(fmt, ...)                 \
    ::Graphite::Core::Logger::Logger::log( \
        ::Graphite::Core::Logger::ELogLevel::Info, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_WARN(fmt, ...)                 \
    ::Graphite::Core::Logger::Logger::log( \
        ::Graphite::Core::Logger::ELogLevel::Warn, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_ERROR(fmt, ...)                \
    ::Graphite::Core::Logger::Logger::log( \
        ::Graphite::Core::Logger::ELogLevel::Error, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_CRITICAL(fmt, ...)                         \
    ::Graphite::Core::Logger::Logger::log(             \
        ::Graphite::Core::Logger::ELogLevel::Critical, \
        __PRETTY_FUNCTION__,                           \
        fmt __VA_OPT__(, ) __VA_ARGS__);               \
    std::this_thread::sleep_for(std::chrono::seconds{2})

#define LOG_DEBUG(fmt, ...)                \
    ::Graphite::Core::Logger::Logger::log( \
        ::Graphite::Core::Logger::ELogLevel::Debug, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_SCOPE(fmt, ...)                                              \
    ::Graphite::Core::Logger::ScopeLogger _graphite_scope_logger         \
    {                                                                    \
        std::format(fmt __VA_OPT__(, ) __VA_ARGS__), __PRETTY_FUNCTION__ \
    }

// TODO: improve?
#define GRAPHITE_ASSERT(condition, message)                                                    \
    do                                                                                         \
    {                                                                                          \
        auto const line{__LINE__ - 2};                                                         \
        if (!(condition))                                                                      \
        {                                                                                      \
            LOG_CRITICAL(                                                                      \
                "Assertion failed on line {}: {} | Condition: {}", line, message, #condition); \
            std::cerr << std::endl                                                             \
                      << "Assertion failed on line " << line << " : " << message               \
                      << " | Condition: " << #condition << std::endl;                          \
            std::abort();                                                                      \
        }                                                                                      \
    } while (0)
#endif
