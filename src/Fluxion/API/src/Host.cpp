/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Data.cpp
/// @author Alexandru Delegeanu
/// @version 0.5
/// @brief Implementation of api related stuff
///

#include "Fluxion/API/LogsPlugin/Host.hpp"

namespace Fluxion::API::LogsPlugin::Host {

LogWriter::LogWriter(IndexToLogRowMap& map) : m_targetMap(map)
{
}

void LogWriter::WriteData(std::size_t const log_index, Bridge::ABI::Span<Bridge::ABI::StringView> const columns)
{
    auto& target_row = m_targetMap[log_index];

    if (target_row.data.size() < columns.size)
    {
        target_row.data.resize(columns.size);
    }

    for (std::size_t column_idx = 0; column_idx < columns.size; ++column_idx)
    {
        target_row.data[column_idx] = columns.data[column_idx];
    }
}

void LogWriter::WriteMetadata(
    std::size_t log_index,
    Graphite::Common::Utility::UniqueID filter_id,
    Graphite::Common::Utility::UniqueID highlight_id)
{
    auto& target_row{m_targetMap[log_index]};

    target_row.metadata.filter_id = filter_id;
    target_row.metadata.highlight_id = highlight_id;
}

} // namespace Fluxion::API::LogsPlugin::Host
