/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file FiltersTabs.cpp
/// @author Alexandru Delegeanu
/// @version 0.4
/// @brief Implementation of @see Fluxion/Data.hpp::Tabs.
///

#include "Fluxion/API/Data.hpp"

namespace Fluxion::API::Data {

namespace Filters {

void Tab::UpdateImGuiID()
{
    imgui_id = name + "###" + id.ToString();
}

} // namespace Filters

namespace Logs {

IndexToLogRowMapWriter::IndexToLogRowMapWriter(IndexToLogRowMap& map) : m_map{map}
{
}

LogRow& IndexToLogRowMapWriter::operator[](std::size_t const index)
{
    return m_map.try_emplace(index).first->second;
}

}; // namespace Logs

} // namespace Fluxion::API::Data
