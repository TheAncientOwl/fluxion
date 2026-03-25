/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogFormatter.hpp
/// @author Alexandru Delegeanu
/// @version 1.1
/// @brief Format helpers.
///

#pragma once

#include <ostream>
#include <string_view>

#include "Graphite/Logger.hpp"

namespace Graphite::Logger::Private::Formatter {

const char* getLevelColor(ELogLevel level) noexcept;
const char* getLevelName(ELogLevel level) noexcept;
const char* getSeparatorColor() noexcept;

void formatScopeColored(std::ostream& os, std::string_view scope, std::string_view color);
void formatScopePlain(std::ostream& os, std::string_view scope);

} // namespace Graphite::Logger::Private::Formatter
