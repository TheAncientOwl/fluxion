/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file TAppAction.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief
///

namespace Graphite::Common::Utility {

template <typename ActionEnum>
struct TAppAction
{
    ActionEnum type;
    std::any payload;
};

} // namespace Graphite::Common::Utility