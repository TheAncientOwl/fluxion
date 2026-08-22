/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Time.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Time utilities
///

#pragma once

#include <chrono>
#include <format>
#include <string>

namespace Graphite::Common::Utility::Time {

template <typename Clock = std::chrono::steady_clock>
std::string FormatDuration(
    std::chrono::time_point<Clock> const& start,
    std::chrono::time_point<Clock> const& end)
{
    auto const duration = end - start;

    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration % std::chrono::hours(1));
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(duration % std::chrono::minutes(1));
    auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration % std::chrono::seconds(1));

    std::string result;
    bool has_started = false;

    if (hours.count() > 0)
    {
        result += std::format("{:02}h ", hours.count());
        has_started = true;
    }

    if (has_started || minutes.count() > 0)
    {
        result += std::format("{:02}m ", minutes.count());
        has_started = true;
    }

    if (has_started || seconds.count() > 0)
    {
        result += std::format("{:02}s ", seconds.count());
        has_started = true;
    }

    // Milliseconds use 3 digits for consistency, or standard 2 if preferred
    result += std::format("{:03}ms", milliseconds.count());

    return result;
}

} // namespace Graphite::Common::Utility::Time