/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogFormatter.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
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
