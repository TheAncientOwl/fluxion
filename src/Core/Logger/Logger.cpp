/// --------------------------------------------------------------------------
///                     Copyright (c) by Graphite 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/graphite/blob/main/LICENSE
///
/// @file Logger.cpp
/// @author Alexandru Delegeanu
/// @version 0.9
/// @brief Implementation of @see Logger.hpp.
///

#include "Logger.hpp"
#include "Ansi.hpp"
#include "LogFormatter.hpp"

#include <ctime>
#include <iomanip>

using namespace std::string_literals;

namespace Graphite::Core::Logger {

static std::mutex g_write_mutex;

Logger& Logger::instance()
{
    static Logger logger{};
    return logger;
}

void Logger::enqueue(LogMessage&& msg)
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

Logger::~Logger()
{
    m_running = false;
    m_cv.notify_one();
    if (m_worker.joinable())
        m_worker.join();
}

std::filesystem::path Logger::GetLogFilePath()
{
    return std::filesystem::current_path() / "app.graphite.log";
}

Logger::Logger()
    : m_running{true}, m_worker{}, m_log_file{Logger::GetLogFilePath(), std::ios::trunc}
{
    if (!m_log_file.is_open())
    {
        std::cerr << "::Graphite::Core::Logger::Logger(): [Critical] failed to open log file: "
                  << GetLogFilePath().string();
        std::terminate();
    }

    m_worker = std::thread(&Logger::processQueue, this);
}

void Logger::processQueue()
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
            LogMessage msg = std::move(m_queue.front());
            m_queue.pop();
            lock.unlock();

            printMessage(msg);

            lock.lock();
        }
    }

    // Flush remaining messages if any
    while (true)
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        if (m_queue.empty())
            break;
        LogMessage msg = std::move(m_queue.front());
        m_queue.pop();
        lock.unlock();

        printMessage(msg);
    }
}

void Logger::printMessage(const LogMessage& msg)
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

    auto const levelColor = Formatter::getLevelColor(msg.level);
    auto const reset = "\033[97m";
    auto const sepColor = Formatter::getSeparatorColor();

    try
    {
        std::lock_guard<std::mutex> lock(g_write_mutex);

        // clang-format off
        std::cout 
            << sepColor << "| " 
            << levelColor << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds << ":" << std::setw(3) << ms << ":" << std::setw(6) << ns
            << sepColor << " | " 
            << levelColor << std::setw(8) << std::setfill(' ') << std::right << Formatter::getLevelName(msg.level)
            << sepColor << " | "
            << levelColor;

        Formatter::formatScopeColored(std::cout, msg.scope, levelColor);

        std::cout << sepColor << " » " << reset << msg.message << std::endl;

        m_log_file
            << "| " 
            << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds << ":" << std::setw(3) << ms << ":" << std::setw(6) << ns
            << " | " 
            << std::setw(8) << std::setfill(' ') << std::right << Formatter::getLevelName(msg.level)
            << " | ";

        Formatter::formatScopePlain(m_log_file, msg.scope);

        m_log_file << " » ";
        Ansi::writeWithoutAnsi(m_log_file, msg.message);
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

ScopeLogger::ScopeLogger(std::string_view const tag, std::string_view const scope)
    : m_tag{tag}, m_scope{scope}, m_start{std::chrono::high_resolution_clock::now()}
{
    static constexpr auto green = "\033[32m";
    static constexpr auto gray = "\033[90m";

    Logger::log(
        LogLevel::Scope, m_scope, "{}[{}+{}]{} Begin {}» {}{}", gray, green, gray, green, gray, green, m_tag);
}

ScopeLogger::~ScopeLogger()
{
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
        LogLevel::Scope,
        m_scope,
        "{}[{}-{}]{} End {}» {}{} ~ elapsed {}",
        gray,
        red,
        gray,
        red,
        gray,
        m_tag,
        gray,
        oss.str());
}

} // namespace Graphite::Core::Logger
