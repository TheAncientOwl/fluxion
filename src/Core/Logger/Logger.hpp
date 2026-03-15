/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Logger.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Logging utilities
///

#pragma once

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
#include <string_view>
#include <thread>

namespace Graphite::Core::Logger {

enum class LogLevel
{
    Trace,
    Info,
    Warn,
    Error,
    Critical,
    Debug,
    Scope
};

struct LogMessage
{
    LogLevel level;
    std::string_view scope;
    std::string message;
    std::chrono::system_clock::time_point time;
};

class Logger
{
public:
    ~Logger();

    template <typename... Args>
    static void log(LogLevel level, std::string_view scope, std::format_string<Args...> fmt, Args&&... args)
    {
        instance().enqueue(
            LogMessage{
                level,
                scope,
                std::format(fmt, std::forward<Args>(args)...),
                std::chrono::system_clock::now()});
    }

private:
    void enqueue(LogMessage&& msg);
    static Logger& instance();

    Logger();
    void processQueue();

    void printMessage(const LogMessage& msg);

    static std::filesystem::path GetLogFilePath();

private:
    std::queue<LogMessage> m_queue;
    std::mutex m_queue_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_running;
    std::thread m_worker;
    std::ofstream m_log_file;
};

class ScopeLogger
{
public:
    ScopeLogger(std::string_view const tag, std::string_view const scope);
    ~ScopeLogger();

private:
    std::string_view m_scope;
    std::string_view m_tag;
    std::chrono::high_resolution_clock::time_point m_start;
};

} // namespace Graphite::Core::Logger

#define LOG_TRACE(fmt, ...)                \
    ::Graphite::Core::Logger::Logger::log( \
        ::Graphite::Core::Logger::LogLevel::Trace, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_INFO(fmt, ...)                 \
    ::Graphite::Core::Logger::Logger::log( \
        ::Graphite::Core::Logger::LogLevel::Info, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_WARN(fmt, ...)                 \
    ::Graphite::Core::Logger::Logger::log( \
        ::Graphite::Core::Logger::LogLevel::Warn, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_ERROR(fmt, ...)                \
    ::Graphite::Core::Logger::Logger::log( \
        ::Graphite::Core::Logger::LogLevel::Error, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_CRITICAL(fmt, ...)             \
    ::Graphite::Core::Logger::Logger::log( \
        ::Graphite::Core::Logger::LogLevel::Critical, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_DEBUG(fmt, ...)                \
    ::Graphite::Core::Logger::Logger::log( \
        ::Graphite::Core::Logger::LogLevel::Debug, __PRETTY_FUNCTION__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define LOG_SCOPE(fmt, ...)                                              \
    ::Graphite::Core::Logger::ScopeLogger _graphite_scope_logger         \
    {                                                                    \
        std::format(fmt __VA_OPT__(, ) __VA_ARGS__), __PRETTY_FUNCTION__ \
    }

// TODO: improve?
#define GRAPHITE_ASSERT(condition, message)                                                \
    do                                                                                     \
    {                                                                                      \
        if (!(condition))                                                                  \
        {                                                                                  \
            LOG_CRITICAL("Assertion failed: {} | Condition: {}", message, #condition);     \
            std::cerr << std::endl                                                         \
                      << "Assertion failed: " << message << " | Condition: " << #condition \
                      << std::endl;                                                        \
            std::abort();                                                                  \
        }                                                                                  \
    } while (0)
