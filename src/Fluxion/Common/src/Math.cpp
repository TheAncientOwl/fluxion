/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Math.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation of @see Math.hpp
///

#include "Fluxion/Common/Utility/Math.hpp"

namespace Fluxion::Common::Utility::Math {

float Percentage(std::size_t const value, std::size_t const total) noexcept
{
    if (total > 0)
    {
        return static_cast<float>(value) / static_cast<float>(total) * 100.0f;
    }
    return 0;
}

} // namespace Fluxion::Common::Utility::Math
