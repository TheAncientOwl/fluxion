/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Ansi.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Implementation of @see Ansii.hpp.
///

#include "Ansi.hpp"

namespace Graphite::Core::Logger::Ansi {

void writeWithoutAnsi(std::ostream& os, std::string_view s) noexcept
{
    size_t idx = 0;
    while (idx < s.size())
    {
        if (s[idx] == '\033' && idx + 1 < s.size() && s[idx + 1] == '[')
        {
            idx += 2; // skip ESC [
            while (idx < s.size() && s[idx] != 'm')
                ++idx;
            if (idx < s.size() && s[idx] == 'm')
                ++idx; // skip final 'm'
        }
        else
        {
            os.put(s[idx++]);
        }
    }
}

} // namespace Graphite::Core::Logger::Ansi
