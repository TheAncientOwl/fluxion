/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Data.hpp
/// @author Alexandru Delegeanu
/// @version 0.13
/// @brief General data.
///

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Graphite/Common/DataStructures/TDoubleBuffer.hpp"
#include "Graphite/Common/Utility/TWithFlags.hpp"
#include "Graphite/Common/Utility/UniqueID.hpp"

#include "Fluxion/API/Data/Common.hpp"
#include "Fluxion/API/LogsPlugin/PluginBridge.hpp"

namespace Fluxion::Application::Data {

namespace Filters {

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
    Fluxion::API::Data::Common::Highlight colors{};
    std::uint8_t priority{};
};

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

// clang-format off
enum class EFiltersMetadataFlag : std::uint8_t {
    None        = 0,      // 00000000
    Applied     = 1 << 0, // 00000001
    SavedToDisk = 1 << 1, // 00000010
};
// clang-format on

struct FiltersGeneralMetadata
    : Graphite::Common::Utility::TWithFlags<FiltersGeneralMetadata, EFiltersMetadataFlag>
{
};

} // namespace Filters

namespace Logs {

struct VisibleLogs
{
    Fluxion::API::LogsPlugin::Data::IndexToLogRowMap logs{};
};

struct SharedFilterMetadata
{
    Fluxion::API::Data::Common::Highlight colors{Fluxion::API::Data::Common::Highlight{
        .foreground = {1.0f, 1.0f, 1.0f, 1.0f},
        .background = {0.0f, 0.0f, 0.0f, 0.0f}}};
};

struct SearchedLog
{
    std::optional<std::size_t> index{std::nullopt};
};

}; // namespace Logs

} // namespace Fluxion::Application::Data
