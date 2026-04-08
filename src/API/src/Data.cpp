/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Data.cpp
/// @author Alexandru Delegeanu
/// @version 0.4
/// @brief Implementation of api related stuff
///

#include "Fluxion/API/LogsPlugin/PluginBridge.hpp"

namespace Fluxion::API {

namespace LogsPlugin::Data {

IndexToLogRowMapWriter::IndexToLogRowMapWriter(IndexToLogRowMap& map) : m_map{map}
{
}

LogRow& IndexToLogRowMapWriter::operator[](std::size_t const index)
{
    return m_map.try_emplace(index).first->second;
}

}; // namespace LogsPlugin::Data

} // namespace Fluxion::API
