/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Data.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief Implementation of @see FluxionData/Data.hpp::Tabs.
///

#include "Fluxion/Data/Data.hpp"

namespace Fluxion::Application::Data {

namespace Filters {

void Tab::UpdateImGuiID()
{
    imgui_id = name + "###" + id.ToString();
}

} // namespace Filters

} // namespace Fluxion::Application::Data
