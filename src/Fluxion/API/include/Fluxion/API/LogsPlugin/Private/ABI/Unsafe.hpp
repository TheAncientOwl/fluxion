/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Unsafe.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief ABI Unsafe data structures
///

#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

#include "imgui.h"

#include "Fluxion/API/LogsPlugin/Private/ABI/Safe.hpp"
#include "Graphite/Common/Utility/TWithFlags.hpp"
#include "Graphite/Common/Utility/UniqueID.hpp"

namespace Fluxion::API::LogsPlugin::Private::ABI::Unsafe {

struct Highlight
{
    ImVec4 foreground{1.0f, 1.0f, 1.0f, 1.0f};
    ImVec4 background{0.0f, 0.0f, 0.0f, 0.0f};
};

struct OnEnableData
{
    std::filesystem::path plugin_home_path{};
};

struct OnDisableData
{
};

struct Condition : public Graphite::Common::Utility::TWithFlags<Condition, Safe::EConditionFlag>
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
    bool highlight_only{false};
};

struct ColumnDetails
{
    Graphite::Common::Utility::UniqueID id{};
    std::string display_name;
};

struct LogRowMetadata
{
    Graphite::Common::Utility::UniqueID filter_id{Graphite::Common::Utility::UniqueID::GetDefault()};
    Graphite::Common::Utility::UniqueID highlight_id{Graphite::Common::Utility::UniqueID::GetDefault()};
};

struct LogRow
{
    std::vector<std::string> data{};
    LogRowMetadata metadata{};
};

using IndexToLogRowMap = std::unordered_map<std::size_t, LogRow>;

} // namespace Fluxion::API::LogsPlugin::Private::ABI::Unsafe
