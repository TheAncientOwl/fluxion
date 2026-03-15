/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file WindowConfiguration.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Configuration of graphite app window.
///

#pragma once

#include <cstdint>
#include <string>

namespace Graphite::Core::Application {

struct WindowConfiguration
{
    std::uint32_t width{0};
    std::uint32_t height{0};
    std::string title{};
};

} // namespace Graphite::Core::Application
