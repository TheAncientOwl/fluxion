/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion/Data.hpp
/// @author Alexandru Delegeanu
/// @version 0.11
/// @brief General data.
///

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Graphite/Common/DataStructures/TDoubleBuffer.hpp"
#include "Graphite/Common/Utility/TWithFlags.hpp"
#include "Graphite/Common/Utility/UniqueID.hpp"

#include "imgui.h"

namespace Fluxion::API::Data {

namespace Filters {

struct Highlight
{
    ImVec4 foreground{};
    ImVec4 background{};
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
    using Ptr = std::shared_ptr<Condition>;

    Graphite::Common::Utility::UniqueID id{};
    Graphite::Common::Utility::UniqueID over_column_id{};
    std::string data{};
};

// clang-format off
enum class EFilterFlag : std::uint8_t
{
    None            = 0,      // 00000000
    IsActive        = 1 << 0, // 00000001
    IsHighlightOnly = 1 << 1, // 00000010
    IsCollapsed     = 1 << 2, // 00000100
};
// clang-format on

struct Filter : public Graphite::Common::Utility::TWithFlags<Filter, EFilterFlag>
{
    using Ptr = std::shared_ptr<Filter>;

    Graphite::Common::Utility::UniqueID id{};
    std::string name{};
    Graphite::Common::DataStructures::TCopyDoubleBuffer<std::vector<Condition::Ptr>> conditions{};
    Highlight colors{};
    std::uint8_t priority{};
};

namespace Active {
struct Condition : public Graphite::Common::Utility::TWithFlags<Condition, EConditionFlag>
{
    std::size_t column_index{};
    std::string data{};
};

struct Filter
{
    Graphite::Common::Utility::UniqueID id{};
    std::vector<Condition> conditions{};
    Highlight colors{};
    std::uint8_t priority{};
};
} // namespace Active

// clang-format off
enum class ETabFlag : std::uint8_t
{
    None     = 0,     // 00000000
    IsActive = 1 << 0 // 00000001
};
// clang-format on

struct Tab : public Graphite::Common::Utility::TWithFlags<Tab, ETabFlag>
{
    using Ptr = std::shared_ptr<Tab>;

    Graphite::Common::Utility::UniqueID id{};
    std::string name{};
    Graphite::Common::DataStructures::TCopyDoubleBuffer<std::vector<Filter::Ptr>> filters{};
    std::string imgui_id{};

    void UpdateImGuiID();
};

} // namespace Filters

namespace Plugin {

struct OnEnableData
{
};

struct OnDisableData
{
};

}; // namespace Plugin

namespace Logs {

struct Range
{
    std::size_t begin{};
    std::size_t end{};
};

// TODO: Add filter ID
struct LogRowMetadata
{
    Filters::Highlight colors{Filters::Highlight{
        .foreground = {1.0f, 1.0f, 1.0f, 1.0f},
        .background = {0.0f, 0.0f, 0.0f, 0.0f}}};
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

struct ColumnDetails
{
    Graphite::Common::Utility::UniqueID id{};
    std::string display_name;
};

}; // namespace Logs

} // namespace Fluxion::API::Data
