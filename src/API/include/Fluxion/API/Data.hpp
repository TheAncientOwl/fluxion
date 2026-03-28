/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Fluxion/Data.hpp
/// @author Alexandru Delegeanu
/// @version 0.6
/// @brief General data.
///

#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "Graphite/Common/TDoubleBufferedVector.hpp"
#include "Graphite/Common/TWithFlags.hpp"
#include "Graphite/Common/UniqueID.hpp"

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

using LogsTableHeader = std::vector<std::string>;

// clang-format off
enum class EFilterComponentFlag : std::uint8_t
{
    None            = 0,      // 00000000
    IsRegex         = 1 << 0, // 00000001
    IsEquals        = 1 << 1, // 00000010
    IsCaseSensitive = 1 << 2, // 00000100
};
// clang-format on

struct FilterComponent : public Graphite::Common::TWithFlags<FilterComponent, EFilterComponentFlag>
{
    Graphite::Common::UniqueID id{};
    Graphite::Common::UniqueID over_field_id{};
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

struct Filter : public Graphite::Common::TWithFlags<Filter, EFilterFlag>
{
    Graphite::Common::UniqueID id{};
    std::string name{};
    Graphite::Common::TDoubleBufferedVector<FilterComponent> components{};
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

struct FiltersTab : public Graphite::Common::TWithFlags<FiltersTab, EFiltersTabFlag>
{
    Graphite::Common::UniqueID id{};
    std::string name{};
    Graphite::Common::TDoubleBufferedVector<Filter> filters{};

    friend std::ostream& operator<<(std::ostream& os, const FiltersTab& v);
};

using FiltersTabs = Graphite::Common::TDoubleBufferedVector<FiltersTab>;

} // namespace Fluxion::API::Data
