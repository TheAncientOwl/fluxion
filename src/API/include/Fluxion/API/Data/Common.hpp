/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Common.hpp
/// @author Alexandru Delegeanu
/// @version 0.12
/// @brief General data.
///

#pragma once

#include "imgui.h"

namespace Fluxion::API::Data::Common {

struct Highlight
{
    ImVec4 foreground{1.0f, 1.0f, 1.0f, 1.0f};
    ImVec4 background{0.0f, 0.0f, 0.0f, 0.0f};
};

} // namespace Fluxion::API::Data::Common
