/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Interface.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Implementation of @see IFluxionLogsPlugin.hpp
///

#include <cstring>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"

namespace Fluxion::API::LogsPlugin {

namespace Adapter {

LogRowMetadata MakeMetadata(UniqueID const filter_id, UniqueID const highlight_filter_id) noexcept
{
    LogRowMetadata metadata{};

    std::memcpy(metadata.filter_id.data, filter_id.Data().data(), sizeof(metadata.filter_id.data));
    std::memcpy(
        metadata.highlight_id.data,
        highlight_filter_id.Data().data(),
        sizeof(metadata.highlight_id.data));

    return metadata;
}

} // namespace Adapter

} // namespace Fluxion::API::LogsPlugin
