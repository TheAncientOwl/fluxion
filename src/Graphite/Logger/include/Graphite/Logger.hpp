/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Logger.hpp
/// @author Alexandru Delegeanu
/// @version 1.9
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
#include <string>
#include <thread>
#include <unordered_map>

#include "imgui.h"

#include "Graphite/Common/TWithFlags.hpp"

namespace Graphite::Logger {

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

struct GlobalLogLevel
{
    GlobalLogLevel(ELogLevel const value, std::string icon, std::string label, ImVec4 const color);
    ELogLevel value{};
    std::string icon{};
    std::string label{};
    std::string display{};
    ImVec4 color{};
};

class Logger
{
public: // Types
    using ScopeEnabledMap = std::unordered_map<std::string, LogScopeFlags>;
    using LogLevels = std::array<GlobalLogLevel, 7>;

public: // Singleton
    static Logger& Instance();
    ~Logger();

public: // API
    template <typename... Args>
    void Log(ELogLevel level, std::string scope, std::format_string<Args...> fmt, Args&&... args)
    {
        {
            std::string key(scope);
            std::lock_guard lock(m_scope_mutex);

            auto it = m_scope_enabled.find(key);
            if (it == m_scope_enabled.end())
            {
                auto flags = GetDefaultScopeFlags();
                m_scope_enabled.emplace(key, flags);
            }
        }

        if (!IsLevelEnabled(level))
        {
            return;
        }

        {
            std::string key(scope);
            std::lock_guard lock(m_scope_mutex);

            auto it = m_scope_enabled.find(key);
            if (it != m_scope_enabled.end())
            {
                if (!it->second[level])
                    return;
            }
        }

        Enqueue(
            LogMessage{
                level,
                scope,
                std::format(fmt, std::forward<Args>(args)...),
                std::chrono::system_clock::now()});
    }

    void SaveConfig();
    void LoadConfig();

    ScopeEnabledMap const& GetScopes() const;
    LogLevels const& GetLevels();

    void SetScopeEnabled(std::string scope, bool const enabled);
    void SetScopeLevelEnabled(std::string scope, ELogLevel const level, bool const enabled);

    void SetLevelState(ELogLevel const level, bool const enabled);
    bool IsLevelEnabled(ELogLevel const level);

private:
    static std::filesystem::path GetConfigFilePath();
    static std::filesystem::path GetLogFilePath();

    Logger();

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

} // namespace Graphite::Logger

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
#define LOG_TRACE(fmt, ...)                     \
    ::Graphite::Logger::Logger::Instance().Log( \
        ::Graphite::Logger::ELogLevel::Trace, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_INFO(fmt, ...)                      \
    ::Graphite::Logger::Logger::Instance().Log( \
        ::Graphite::Logger::ELogLevel::Info, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_WARN(fmt, ...)                      \
    ::Graphite::Logger::Logger::Instance().Log( \
        ::Graphite::Logger::ELogLevel::Warn, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_ERROR(fmt, ...)                     \
    ::Graphite::Logger::Logger::Instance().Log( \
        ::Graphite::Logger::ELogLevel::Error, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_CRITICAL(fmt, ...)                                                                         \
    ::Graphite::Logger::Logger::Instance().Log(                                                        \
        ::Graphite::Logger::ELogLevel::Critical, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__); \
    std::this_thread::sleep_for(std::chrono::seconds{2})

#define LOG_DEBUG(fmt, ...)                     \
    ::Graphite::Logger::Logger::Instance().Log( \
        ::Graphite::Logger::ELogLevel::Debug, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_SCOPE(fmt, ...)                                              \
    ::Graphite::Logger::ScopeLogger _graphite_scope_logger               \
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
