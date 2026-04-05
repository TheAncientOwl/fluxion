/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file AppState.hpp
/// @author Alexandru Delegeanu
/// @version 0.13
/// @brief Application state.
///

#pragma once

#include <memory>

#include "Fluxion/API/Data.hpp"
#include "Fluxion/API/IFluxionPlugin.hpp"
#include "Graphite/Common/DataStructures/TDoubleBuffer.hpp"

namespace Fluxion::Application {

enum class EFluxionAction : std::uint8_t
{
    None = 0,
    FilterAction = 1,
    LogsViewLayerAction = 2
};

namespace Internal {

struct VisibleLogsChunk
{
    Fluxion::API::Data::Logs::IndexToLogRowMap logs{};
    std::size_t filled_size{};
};

// clang-format off
enum class EFiltersMetadataFlag : std::uint8_t {
    None        = 0,      // 00000000
    Applied     = 1 << 0, // 00000001
    SavedToDisk = 1 << 1, // 00000010
};
// clang-format on

struct FiltersMetadata : Graphite::Common::Utility::TWithFlags<FiltersMetadata, EFiltersMetadataFlag>
{
};

} // namespace Internal

struct AppState
{
    std::unique_ptr<Fluxion::API::IFluxionPlugin> logs_logic{nullptr};

    struct
    {
        Graphite::Common::DataStructures::TCopyDoubleBuffer<std::vector<Fluxion::API::Data::Filters::Tab::Ptr>>
            tabs{};
        Graphite::Common::DataStructures::TCopyLockingDoubleBuffer<Internal::FiltersMetadata> metadata;
    } filters{};

    struct
    {
        bool logs_view{true};
        bool debug{true};
        bool filters{true};
    } layers_active{};

    struct
    {
        Graphite::Common::DataStructures::TSwapDoubleBuffer<Internal::VisibleLogsChunk> visible_chunk{};
        std::vector<Fluxion::API::Data::Logs::ColumnDetails> table_header{};
    } logs{};
};

namespace DefaultState {

AppState Make();

std::vector<Fluxion::API::Data::Filters::Tab::Ptr> MakeDefaultTabs();

} // namespace DefaultState

} // namespace Fluxion::Application
