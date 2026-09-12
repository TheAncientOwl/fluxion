/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Host.hpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief General data.
///

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Bridge.hpp"
#include "Graphite/Common/Utility/UniqueID.hpp"

namespace Fluxion::API::LogsPlugin::Host {

struct ColumnDetails
{
    Graphite::Common::Utility::UniqueID id{};
    std::string display_name{};

    ColumnDetails() = default;
    inline ColumnDetails(Graphite::Common::Utility::UniqueID id, std::string display_name)
        : id(id), display_name(std::move(display_name))
    {
    }
    inline ColumnDetails(Bridge::ColumnDetails const& details)
        : id(details.id), display_name(details.display_name)
    {
    }
};

struct LogRow
{
    std::vector<std::string> data{};
    Bridge::LogRowMetadata metadata{};
};

using IndexToLogRowMap = std::unordered_map<std::size_t, LogRow>;

class LogWriter final : public Bridge::ILogsWriter
{
public:
    explicit LogWriter(IndexToLogRowMap& map);

    void WriteData(
        std::size_t const log_index,
        Bridge::ABI::Span<Bridge::ABI::StringView> const columns) override;

    void WriteMetadata(
        std::size_t const log_index,
        Graphite::Common::Utility::UniqueID const filter_id,
        Graphite::Common::Utility::UniqueID const highlight_id) override;

private:
    IndexToLogRowMap& m_targetMap;
};

} // namespace Fluxion::API::LogsPlugin::Host
