/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogFormatter.cpp
/// @author Alexandru Delegeanu
/// @version 1.1
/// @brief Implementation of @see LogFormatter.hpp.
///

#include "LogFormatter.hpp"
#include "Ansi.hpp"

namespace Graphite::Core::Logger::Formatter {

const char* getLevelColor(ELogLevel level) noexcept
{
    switch (level)
    {
    case ELogLevel::Trace:
        return "\033[97m";
    case ELogLevel::Info:
        return "\033[34m";
    case ELogLevel::Warn:
        return "\033[33m";
    case ELogLevel::Error:
        return "\033[31m";
    case ELogLevel::Critical:
        return "\033[91m";
    case ELogLevel::Debug:
        return "\033[92m";
    case ELogLevel::Scope:
        return "\033[90m";
    default:
        return "\033[97m";
    }
}

const char* getLevelName(ELogLevel level) noexcept
{
    switch (level)
    {
    case ELogLevel::Trace:
        return "trace";
    case ELogLevel::Info:
        return "info";
    case ELogLevel::Warn:
        return "warn";
    case ELogLevel::Error:
        return "error";
    case ELogLevel::Critical:
        return "critical";
    case ELogLevel::Debug:
        return "debug";
    case ELogLevel::Scope:
        return "scope";
    default:
        return "unknown";
    }
}

const char* getSeparatorColor() noexcept
{
    return "\033[90m";
}

void formatScopeColored(std::ostream& os, std::string_view scope, std::string_view color)
{
    if (scope.empty())
        return;

    std::string_view name = scope;

    auto space = name.find(' ');
    if (space != std::string_view::npos)
    {
        auto possibleName = name.substr(space + 1);
        if (possibleName.find('(') != std::string_view::npos)
            name = possibleName;
    }

    auto firstScope = name.find("::");
    if (firstScope != std::string_view::npos)
    {
        auto s = name.rfind(' ', firstScope);
        if (s != std::string_view::npos)
            name.remove_prefix(s + 1);
    }

    auto openParen = name.find('(');
    if (openParen != std::string_view::npos)
        name = name.substr(0, openParen);

    static constexpr auto gray = "\033[90m";

    size_t start = 0;
    while (true)
    {
        auto pos = name.find("::", start);
        if (pos == std::string_view::npos)
        {
            os << color << name.substr(start);
            break;
        }

        os << color << name.substr(start, pos - start);
        os << gray << "::";
        start = pos + 2;
    }
}

void formatScopePlain(std::ostream& os, std::string_view scope)
{
    if (scope.empty())
        return;

    std::string_view name = scope;

    auto space = name.find(' ');
    if (space != std::string_view::npos)
    {
        auto possibleName = name.substr(space + 1);
        if (possibleName.find('(') != std::string_view::npos)
            name = possibleName;
    }

    auto firstScope = name.find("::");
    if (firstScope != std::string_view::npos)
    {
        auto s = name.rfind(' ', firstScope);
        if (s != std::string_view::npos)
            name.remove_prefix(s + 1);
    }

    auto openParen = name.find('(');
    if (openParen != std::string_view::npos)
        name = name.substr(0, openParen);

    Ansi::writeWithoutAnsi(os, name);
}

} // namespace Graphite::Core::Logger::Formatter
