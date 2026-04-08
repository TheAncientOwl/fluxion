/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file PluginBridgeData.hpp
/// @author Alexandru Delegeanu
/// @version 0.12
/// @brief General data.
///

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Graphite/Common/Utility/TWithFlags.hpp"
#include "Graphite/Common/Utility/UniqueID.hpp"

#include "Fluxion/API/Data/Common.hpp"

namespace Fluxion::API::LogsPlugin::Data {

struct OnEnableData
{
};

struct OnDisableData
{
};

// clang-format off
enum class EConditionFlag : std::uint8_t
{
    None            = 0,      // 00000000
    IsRegex         = 1 << 0, // 00000001
    IsEquals        = 1 << 1, // 00000010
    IsCaseSensitive = 1 << 2, // 00000100
};
// clang-format on

struct Condition : public Graphite::Common::Utility::TWithFlags<Condition, EConditionFlag>
{
    std::size_t column_index{};
    std::string data{};
};

struct Filter
{
    Graphite::Common::Utility::UniqueID id{};
    std::vector<Condition> conditions{};
    Fluxion::API::Data::Common::Highlight colors{};
    std::uint8_t priority{};
    bool highlight_only{false};
};

struct ColumnDetails
{
    Graphite::Common::Utility::UniqueID id{};
    std::string display_name;
};

struct Range
{
    std::size_t begin{};
    std::size_t end{};
};

struct LogRowMetadata
{
    Graphite::Common::Utility::UniqueID filter_id{Graphite::Common::Utility::UniqueID::Default()};
    Graphite::Common::Utility::UniqueID highlight_id{Graphite::Common::Utility::UniqueID::Default()};
};

struct LogRow
{
    std::vector<std::string> data{};
    LogRowMetadata metadata{};
};

using IndexToLogRowMap = std::unordered_map<std::size_t, LogRow>;

class IndexToLogRowMapWriter
{
public:
    explicit IndexToLogRowMapWriter(IndexToLogRowMap& map);

    LogRow& operator[](std::size_t const index);

private:
    IndexToLogRowMap& m_map;
};

} // namespace Fluxion::API::LogsPlugin::Data
