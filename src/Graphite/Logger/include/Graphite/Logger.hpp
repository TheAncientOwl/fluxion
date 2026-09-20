/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Logger.hpp
/// @author Alexandru Delegeanu
/// @version 1.20
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
#include "Graphite/Settings/PersistentSettings.hpp"

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

namespace std {

template <>
struct formatter<std::filesystem::path> : formatter<std::string_view>
{
    auto format(const std::filesystem::path& p, format_context& ctx) const
    {
        return formatter<std::string_view>::format(p.string(), ctx);
    }
};

} // namespace std

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

    [[nodiscard]] size_t operator()(const char* str) const
    {
        return std::hash<std::string_view>{}(str);
    }
};

class GRAPHITE_LOGGER_API Logger
{
public: // Types
    using ScopeEnabledMap =
        std::unordered_map<std::string, LogScopeFlags, StringHash, std::equal_to<>>;
    using LogLevels = std::array<GlobalLogLevel, 7>;

public: // API
    Logger();
    ~Logger();

    // ABI-Safe methods that cross the boundary using only raw C types
    const char* DefineLogScope(const char* scope);
    void LogRaw(ELogLevel level, const char* scope, const char* message);
    bool IsScopeLevelEnabledRaw(const char* scope, ELogLevel level);

    template <typename... Args>
    void Log(ELogLevel level, const char* scope, std::format_string<Args...> fmt, Args&&... args)
    {
        if (!IsLevelEnabled(level))
        {
            return;
        }

        // Check if the specific scope level is enabled via the safe ABI boundary
        if (!IsScopeLevelEnabledRaw(scope, level))
        {
            return;
        }

        // Format is executed completely locally in the caller's standard library.
        // We decay the resulting std::string to a raw const char* before crossing the boundary.
        std::string formatted_message = std::format(fmt, std::forward<Args>(args)...);
        LogRaw(level, scope, formatted_message.c_str());
    }

    void SaveConfig();
    void LoadConfig();

    ///
    /// @brief Returns a thread-safe snapshot copy to prevent iterator invalidation and data races during UI iteration
    ///
    ScopeEnabledMap GetScopes() const;
    static LogLevels const& GetLevels();

    void SetScopeEnabled(std::string_view scope, bool const enabled);
    void SetScopeLevelEnabled(std::string_view scope, ELogLevel const level, bool const enabled);

    void SetLevelState(ELogLevel const level, bool const enabled);
    [[nodiscard]] bool IsLevelEnabled(ELogLevel const level) const noexcept;

    void Shutdown();

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

    mutable std::mutex m_scope_mutex;
    ScopeEnabledMap m_scope_enabled;
    std::atomic<uint8_t> m_global_level_mask;

    std::vector<std::string> m_queued_defined_scopes{};
    Graphite::Settings::PersistentSettings m_settings;
};

// Moved fully inline so standard library types (std::string, std::chrono)
// never cross the DLL/.so boundary.
class ScopeLogger
{
public:
    inline ScopeLogger(std::string tag, const char* scope)
        : m_scope{scope}, m_tag{std::move(tag)}, m_start{}
    {
        auto& logger = Graphite::Logger::GetLogger();
        if (!logger.IsLevelEnabled(ELogLevel::Scope))
            return;
        if (!logger.IsScopeLevelEnabledRaw(m_scope, ELogLevel::Scope))
            return;

        m_start = std::chrono::high_resolution_clock::now();

        static constexpr auto green = "\033[32m";
        static constexpr auto gray = "\033[90m";

        std::string msg =
            std::format("{}[{}+{}]{} Begin {}» {}{}", gray, green, gray, green, gray, green, m_tag);
        logger.LogRaw(ELogLevel::Scope, m_scope, msg.c_str());
    }

    inline ~ScopeLogger()
    {
        auto& logger = Graphite::Logger::GetLogger();
        if (!logger.IsLevelEnabled(ELogLevel::Scope))
            return;
        if (!logger.IsScopeLevelEnabledRaw(m_scope, ELogLevel::Scope))
            return;

        auto const end = std::chrono::high_resolution_clock::now();
        auto const elapsed = end - m_start;

        auto const hours = std::chrono::duration_cast<std::chrono::hours>(elapsed);
        auto const minutes = std::chrono::duration_cast<std::chrono::minutes>(elapsed - hours);
        auto const seconds =
            std::chrono::duration_cast<std::chrono::seconds>(elapsed - hours - minutes);
        auto const milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(elapsed - hours - minutes - seconds);
        auto const nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
            elapsed - hours - minutes - seconds - milliseconds);

        static constexpr auto red = "\033[91m";
        static constexpr auto gray = "\033[90m";
        static constexpr auto reset = "\033[97m";

        std::string time_str;
        bool started = false;

        if (hours.count() > 0)
        {
            time_str += std::to_string(hours.count()) + "h";
            started = true;
        }
        if (started || minutes.count() > 0)
        {
            if (started)
                time_str += ", ";
            time_str += std::to_string(minutes.count()) + "m";
            started = true;
        }
        if (started || seconds.count() > 0)
        {
            if (started)
                time_str += ", ";
            time_str += std::to_string(seconds.count()) + "s";
            started = true;
        }
        if (started || milliseconds.count() > 0)
        {
            if (started)
                time_str += ", ";
            time_str += std::to_string(milliseconds.count()) + "ms";
            started = true;
        }
        if (started || nanoseconds.count() > 0)
        {
            if (started)
                time_str += ", ";
            time_str += std::to_string(nanoseconds.count()) + "ns";
        }
        time_str += reset;

        std::string msg = std::format(
            "{}[{}-{}]{} End   {}» {}{} ~ elapsed {}", gray, red, gray, red, gray, m_tag, gray, time_str);

        logger.LogRaw(ELogLevel::Scope, m_scope, msg.c_str());
    }

private:
    const char* m_scope;
    std::string m_tag;
    std::chrono::high_resolution_clock::time_point m_start;
};

inline void DisableAllScopes()
{
    auto& logger = Graphite::Logger::GetLogger();
    for (auto const& level_info : Graphite::Logger::Logger::GetLevels())
    {
        logger.SetLevelState(level_info.value, false);
    }
}

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

#define USE_LOG_SCOPE(scope) [[maybe_unused]] auto const __graphite_log_scope = #scope

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

#define LOG_SCOPE(fmt, ...)                                                  \
    ::Graphite::Logger::ScopeLogger CONCAT(_graphite_scope_logger, __LINE__) \
    {                                                                        \
        std::format(fmt __VA_OPT__(, ) __VA_ARGS__), __graphite_log_scope    \
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
