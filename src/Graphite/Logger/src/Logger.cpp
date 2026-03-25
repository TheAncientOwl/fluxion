/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Logger.cpp
/// @author Alexandru Delegeanu
/// @version 1.9
/// @brief Implementation of @see Logger.hpp.
///

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "IconsCodicons.h"

#include "Ansi.hpp"
#include "Graphite/Logger.hpp"
#include "LogFormatter.hpp"

using namespace std::string_literals;

namespace Graphite::Logger {

static std::mutex g_write_mutex;

Logger::ScopeEnabledMap Logger::s_scope_enabled{};
std::mutex Logger::s_scope_mutex{};

Logger& Logger::Instance()
{
    static Logger logger{};
    return logger;
}

Logger::LogLevel::LogLevel(ELogLevel const p_level, std::string p_icon, std::string p_label, ImVec4 const p_color)
    : value{p_level}, icon{std::move(p_icon)}, label{std::move(p_label)}, color{p_color}
{
    this->display = this->icon + " " + this->label;
}

void Logger::SaveConfig()
{
    LOG_SCOPE("");
    auto const config_path = GetConfigFilePath();
    std::ofstream ofs(config_path, std::ios::trunc);
    if (!ofs.is_open())
    {
#ifdef _WIN32
        LOG_ERROR("Failed to open config file {}", config_path.string());
#else
        LOG_ERROR("Failed to open config file {}", config_path.c_str());
#endif
        return;
    }

    {
        std::lock_guard lock{s_scope_mutex};

        for (auto const& [scope, enabled] : s_scope_enabled)
        {
            ofs << static_cast<int>(enabled.GetStorage()) << " " << scope << "\n";
        }
    }
}

void Logger::LoadConfig()
{
    auto const config_path = GetConfigFilePath();
    std::ifstream ifs(config_path);
    if (!ifs.is_open())
    {
        // No config file, silently return
        return;
    }

    std::string line{};
    std::string state{};
    std::string name{};

    std::lock_guard lock{s_scope_mutex};
    while (std::getline(ifs, line))
    {
        std::istringstream iss(line);

        if (!(iss >> state))
        {
            continue;
        }

        std::getline(iss, name);
        auto const start = name.find_first_not_of(" \t");
        if (start != std::string::npos)
        {
            name = name.substr(start);
        }
        else
        {
            name.clear();
        }

        LogScopeFlags flags = GetDefaultScopeFlags();
        int storage{0};
        try
        {
            storage = std::stoi(state);
        }
        catch (...)
        {
            continue;
        }

        flags.SetStorage(static_cast<LogScopeFlags::Storage>(storage));

        s_scope_enabled[std::move(name)] = flags;
    }
}

std::filesystem::path Logger::GetConfigFilePath()
{
    // TODO: Move home path to general utils
    const char* home = std::getenv("HOME");
    if (!home)
    {
        home = std::getenv("USERPROFILE"); // Windows fallback
        if (!home)
        {
            throw std::runtime_error("Cannot determine home directory for config file");
        }
    }
    std::filesystem::path config_dir = std::filesystem::path(home) / ".fluxion";
    std::error_code ec;
    std::filesystem::create_directories(config_dir, ec);
    if (ec)
    {
        std::cerr << "Failed to create config directory: " << config_dir
                  << " error: " << ec.message() << std::endl;
    }
    return config_dir / "app.graphite.logger.cfg";
}

void Logger::Enqueue(LogMessage&& msg)
{
    if (!m_running)
    {
        return;
    }

    try
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_queue.emplace(std::move(msg));
    }
    catch (const std::system_error& e)
    {
        std::cerr << "Logger::enqueue - mutex lock failed: " << e.what()
                  << " addr=" << &m_queue_mutex << " this=" << this << "\n";
        std::terminate();
    }

    m_cv.notify_one();
}

LogScopeFlags Logger::GetDefaultScopeFlags()
{
    LogScopeFlags flags;
    for (auto const& log_level : GetLevels())
    {
        flags[log_level.value] = true;
    }

    return flags;
}

std::string const& Logger::GetGlobalScopeKey()
{
    static const std::string key{"__graphite.global.scopes__"};
    return key;
}

void Logger::SetLevelState(ELogLevel const level, bool const enabled)
{
    std::lock_guard lock{s_scope_mutex};
    auto& flags = s_scope_enabled[GetGlobalScopeKey()];
    if (flags.GetStorage() == 0)
    {
        flags = GetDefaultScopeFlags();
    }

    flags[level] = enabled;
}

bool Logger::IsLevelEnabled(ELogLevel level)
{
    std::lock_guard lock{s_scope_mutex};
    auto it = s_scope_enabled.find(GetGlobalScopeKey());
    if (it == s_scope_enabled.end())
    {
        auto const flags = GetDefaultScopeFlags();
        s_scope_enabled.emplace(GetGlobalScopeKey(), flags);
        return flags[level];
    }

    return it->second[level];
}

void Logger::SetScopeEnabled(std::string scope, bool enabled)
{
    std::lock_guard lock{s_scope_mutex};
    auto& flags = s_scope_enabled[std::move(scope)];
    if (flags.GetStorage() == 0)
    {
        flags = GetDefaultScopeFlags();
    }

    for (auto const& log_level : GetLevels())
    {
        flags[log_level.value] = enabled;
    }
}

void Logger::SetScopeLevelState(std::string scope, ELogLevel const level, bool const enabled)
{
    std::lock_guard lock{s_scope_mutex};
    auto& flags = s_scope_enabled[std::move(scope)];

    flags[level] = enabled;
}

Logger::ScopeEnabledMap Logger::GetScopes()
{
    std::lock_guard lock{s_scope_mutex};
    auto scopes = s_scope_enabled;
    scopes.erase(GetGlobalScopeKey());
    return scopes;
}

Logger::LogLevels const& Logger::GetLevels()
{
    static LogLevels s_levels{
        LogLevel(ELogLevel::Scope, ICON_CI_SEARCH, "Scope", {0.75f, 0.75f, 0.25f, 0.60f}),
        LogLevel(ELogLevel::Debug, ICON_CI_DEBUG, "Debug", {0.50f, 0.75f, 0.50f, 0.60f}),
        LogLevel(ELogLevel::Trace, ICON_CI_SURROUND_WITH, "Trace", {0.50f, 0.50f, 0.50f, 0.60f}),
        LogLevel(ELogLevel::Info, ICON_CI_INFO, "Info", {0.00f, 0.75f, 1.00f, 0.60f}),
        LogLevel(ELogLevel::Warn, ICON_CI_WARNING, "Warn", {1.00f, 0.65f, 0.00f, 0.60f}),
        LogLevel(ELogLevel::Error, ICON_CI_ERROR, "Error", {1.00f, 0.25f, 0.25f, 0.60f}),
        LogLevel(ELogLevel::Critical, ICON_CI_CIRCLE_SLASH, "Critical", {0.75f, 0.00f, 0.00f, 0.60f}),
    };
    return s_levels;
}

Logger::~Logger()
{
    m_running = false;
    m_cv.notify_one();
    if (m_worker.joinable())
        m_worker.join();
}

std::filesystem::path Logger::GetLogFilePath()
{
    // TODO: Move home path to general utils
    const char* home = std::getenv("HOME");
    if (!home)
    {
        home = std::getenv("USERPROFILE"); // Windows fallback
        if (!home)
        {
            throw std::runtime_error("Cannot determine home directory for log file");
        }
    }
    std::filesystem::path log_dir = std::filesystem::path(home) / ".fluxion";
    std::error_code ec;
    std::filesystem::create_directories(log_dir, ec);
    if (ec)
    {
        std::cerr << "Failed to create log directory: " << log_dir << " error: " << ec.message()
                  << std::endl;
    }
    return log_dir / "app.graphite.log";
}

Logger::Logger()
    : m_running{true}, m_worker{}, m_log_file{Logger::GetLogFilePath(), std::ios::trunc}
{
    if (!m_log_file.is_open())
    {
        std::cerr << "::Graphite::Logger::Logger(): [Critical] failed to open log file: "
                  << GetLogFilePath().string();
        std::terminate();
    }

    m_worker = std::thread(&Logger::ProcessQueue, this);
}

void Logger::ProcessQueue()
{
    while (m_running)
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        try
        {
            m_cv.wait(lock, [this] { return !m_queue.empty() || !m_running; });
        }
        catch (const std::system_error& e)
        {
            std::cerr << "Logger::processQueue - condition_variable wait failed: " << e.what()
                      << " m_queue_mutex=" << &m_queue_mutex << " this=" << this << "\n";
            std::terminate();
        }

        while (!m_queue.empty())
        {
            auto const msg = std::move(m_queue.front());
            m_queue.pop();
            lock.unlock();

            PrintMessage(msg);

            lock.lock();
        }
    }

    // Flush remaining messages if any
    while (true)
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        if (m_queue.empty())
            break;
        auto const msg = std::move(m_queue.front());
        m_queue.pop();
        lock.unlock();

        PrintMessage(msg);
    }
}

void Logger::PrintMessage(const LogMessage& msg)
{
    // Format: | HH:MM:SS:ms:ns | LEVEL | Scope::Subscope: Message

    auto const duration_since_epoch = msg.time.time_since_epoch();
    auto const seconds_since_epoch =
        std::chrono::duration_cast<std::chrono::seconds>(duration_since_epoch);
    auto const milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        duration_since_epoch - seconds_since_epoch);
    auto const nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
        duration_since_epoch - seconds_since_epoch - milliseconds);
    auto const seconds_in_day = seconds_since_epoch.count() % 86400;

    auto const hours = static_cast<int>(seconds_in_day / 3600);
    auto const minutes = static_cast<int>((seconds_in_day % 3600) / 60);
    auto const seconds = static_cast<int>(seconds_in_day % 60);
    auto const ms = static_cast<int>(milliseconds.count());
    auto const ns = static_cast<int>(nanoseconds.count());

    auto const levelColor = Private::Formatter::getLevelColor(msg.level);
    auto const reset = "\033[97m";
    auto const sepColor = Private::Formatter::getSeparatorColor();

    try
    {
        std::lock_guard<std::mutex> lock(g_write_mutex);

        // clang-format off
        std::cout 
            << sepColor << "| " 
            << levelColor << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds << ":" << std::setw(3) << ms << ":" << std::setw(6) << ns
            << sepColor << " | " 
            << levelColor << std::setw(8) << std::setfill(' ') << std::right << Private::Formatter::getLevelName(msg.level)
            << sepColor << " | "
            << levelColor;

        Private::Formatter::formatScopeColored(std::cout, msg.scope, levelColor);

        std::cout << sepColor << " » " << reset << msg.message << std::endl;

        m_log_file
            << "| " 
            << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds << ":" << std::setw(3) << ms << ":" << std::setw(6) << ns
            << " | " 
            << std::setw(8) << std::setfill(' ') << std::right << Private::Formatter::getLevelName(msg.level)
            << " | ";

        Private::Formatter::formatScopePlain(m_log_file, msg.scope);

        m_log_file << " » ";
        Private::Ansi::writeWithoutAnsi(m_log_file, msg.message);
        m_log_file << std::endl;
        // clang-format on
    }
    catch (const std::system_error& e)
    {
        std::cerr << "Logger::printMessage - write_mutex lock failed: " << e.what()
                  << " addr=" << &g_write_mutex << " this=" << this << "\n";
        std::terminate();
    }
}

ScopeLogger::ScopeLogger(std::string tag, std::string scope)
    : m_scope{scope}, m_tag{std::move(tag)}, m_start{}
{
    if (!Logger::IsLevelEnabled(ELogLevel::Scope))
        return;

    m_start = std::chrono::high_resolution_clock::now();

    static constexpr auto green = "\033[32m";
    static constexpr auto gray = "\033[90m";

    Logger::log(
        ELogLevel::Scope, m_scope, "{}[{}+{}]{} Begin {}» {}{}", gray, green, gray, green, gray, green, m_tag);
}

ScopeLogger::~ScopeLogger()
{
    if (!Logger::IsLevelEnabled(ELogLevel::Scope))
        return;

    auto const end = std::chrono::high_resolution_clock::now();
    auto const elapsed = end - m_start;

    auto const hours = std::chrono::duration_cast<std::chrono::hours>(elapsed);
    auto const minutes = std::chrono::duration_cast<std::chrono::minutes>(elapsed - hours);
    auto const seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed - hours - minutes);
    auto const milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed - hours - minutes - seconds);
    auto const nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(
        elapsed - hours - minutes - seconds - milliseconds);

    static constexpr auto red = "\033[91m";
    static constexpr auto gray = "\033[90m";
    static constexpr auto reset = "\033[97m";

    std::ostringstream oss;
    oss << std::setfill('0');

    bool started = false;
    if (hours.count() > 0)
    {
        oss << hours.count() << "h";
        started = true;
    }
    if (started || minutes.count() > 0)
    {
        if (started)
        {
            oss << ", ";
        }
        oss << minutes.count() << "m";
        started = true;
    }
    if (started || seconds.count() > 0)
    {
        if (started)
        {
            oss << ", ";
        }
        oss << seconds.count() << "s";
        started = true;
    }
    if (started || milliseconds.count() > 0)
    {
        if (started)
        {
            oss << ", ";
        }
        oss << milliseconds.count() << "ms";
        started = true;
    }
    if (started || nanoseconds.count() > 0)
    {
        if (started)
        {
            oss << ", ";
        }
        oss << nanoseconds.count() << "ns";
    }
    oss << reset;

    Logger::log(
        ELogLevel::Scope,
        m_scope,
        "{}[{}-{}]{} End   {}» {}{} ~ elapsed {}",
        gray,
        red,
        gray,
        red,
        gray,
        m_tag,
        gray,
        oss.str());
}

} // namespace Graphite::Logger
