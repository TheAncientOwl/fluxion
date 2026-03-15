/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Ansi.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief ANSI helpers for escape sequences.
///

#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace Graphite::Core::Logger::Ansi {

void writeWithoutAnsi(std::ostream& os, std::string_view s) noexcept;

} // namespace Graphite::Core::Logger::Ansi
