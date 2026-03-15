/// --------------------------------------------------------------------------
///                     Copyright (c) by Graphite 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/graphite/blob/main/LICENSE
///
/// @file LogFormatter.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Format helpers.
///

#pragma once

#include <ostream>
#include <string_view>
#include "Logger.hpp"

namespace Graphite::Core::Logger::Formatter {

const char* getLevelColor(LogLevel level) noexcept;
const char* getLevelName(LogLevel level) noexcept;
const char* getSeparatorColor() noexcept;

void formatScopeColored(std::ostream& os, std::string_view scope, std::string_view color);
void formatScopePlain(std::ostream& os, std::string_view scope);

} // namespace Graphite::Core::Logger::Formatter
