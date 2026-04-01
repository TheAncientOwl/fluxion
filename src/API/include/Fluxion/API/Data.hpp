/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion/Data.hpp
/// @author Alexandru Delegeanu
/// @version 0.9
/// @brief General data.
///

#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "Graphite/Common/DataStructures/TDoubleBuffer.hpp"
#include "Graphite/Common/Utility/TWithFlags.hpp"
#include "Graphite/Common/Utility/UniqueID.hpp"

#include "imgui.h"

namespace Fluxion::API::Data {

namespace Plugin {

struct OnEnableData
{
};

struct OnDisableData
{
};

}; // namespace Plugin

struct LogsTableColumnDetails
{
    Graphite::Common::Utility::UniqueID id{};
    std::string display_name;
};

// clang-format off
enum class EFilterComponentFlag : std::uint8_t
{
    None            = 0,      // 00000000
    IsRegex         = 1 << 0, // 00000001
    IsEquals        = 1 << 1, // 00000010
    IsCaseSensitive = 1 << 2, // 00000100
};
// clang-format on

struct FilterComponent
    : public Graphite::Common::Utility::TWithFlags<FilterComponent, EFilterComponentFlag>
{
    using Ptr = std::shared_ptr<FilterComponent>;

    Graphite::Common::Utility::UniqueID id{};
    Graphite::Common::Utility::UniqueID over_field_id{};
    std::string data{};

    friend std::ostream& operator<<(std::ostream& os, const FilterComponent& v);
};

struct FilterColors
{
    ImVec4 foreground{};
    ImVec4 background{};
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
    Graphite::Common::DataStructures::TCopyDoubleBuffer<std::vector<FilterComponent::Ptr>> components{};
    FilterColors colors{};
    std::uint8_t priority{};

    friend std::ostream& operator<<(std::ostream& os, const Filter& v);
};

// clang-format off
enum class EFiltersTabFlag : std::uint8_t
{
    None     = 0,     // 00000000
    IsActive = 1 << 0 // 00000001
};
// clang-format on

struct FiltersTab : public Graphite::Common::Utility::TWithFlags<FiltersTab, EFiltersTabFlag>
{
    using Ptr = std::shared_ptr<FiltersTab>;

    Graphite::Common::Utility::UniqueID id{};
    std::string name{};
    Graphite::Common::DataStructures::TCopyDoubleBuffer<std::vector<Filter::Ptr>> filters{};
    std::string imgui_id{};

    void UpdateImGuiID();

    friend std::ostream& operator<<(std::ostream& os, const FiltersTab& v);
};

} // namespace Fluxion::API::Data
