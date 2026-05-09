/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file AppState.hpp
/// @author Alexandru Delegeanu
/// @version 0.17
/// @brief Application state.
///

#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>

#include "Fluxion/API/LogsPlugin/IFluxionLogsPlugin.hpp"
#include "Fluxion/Data/Data.hpp"
#include "Graphite/Common/DataStructures/TDoubleBuffer.hpp"
#include "Graphite/Common/Plugin/DynamicLibrary.hpp"

namespace Fluxion::Application {

enum class EFluxionAction : std::uint8_t
{
    None = 0,
    FilterAction = 1,
    LogsViewLayerAction = 2
};

struct AppState
{
    std::unique_ptr<Fluxion::API::LogsPlugin::IFluxionLogsPlugin> logs_plugin{nullptr};
    std::filesystem::path selected_logs_plugin_path{};
    std::unique_ptr<Graphite::Common::Plugin::DynamicLibrary> loaded_plugin_library{nullptr};

    using IdToMetadataMap =
        std::unordered_map<Graphite::Common::Utility::UniqueID, Data::Logs::SharedFilterMetadata>;
    using IdToMetadataMapUpdates =
        std::vector<std::pair<Graphite::Common::Utility::UniqueID, Data::Logs::SharedFilterMetadata>>;
    struct
    {
        Graphite::Common::DataStructures::TCopyDoubleBuffer<std::vector<Data::Filters::Tab::Ptr>> tabs{};
        Graphite::Common::DataStructures::TCopyLockingDoubleBuffer<Data::Filters::FiltersGeneralMetadata> metadata;

        Graphite::Common::DataStructures::TSwapDoubleBuffer<IdToMetadataMapUpdates> id_to_metadata_updates{};
        IdToMetadataMap id_to_metadata{};

    } filters{};

    struct
    {
        bool logs_view{true};
        bool debug{true};
        bool filters{true};
        bool settings{true};
    } layers_active{};

    struct
    {
        Graphite::Common::DataStructures::TSwapDoubleBuffer<Data::Logs::VisibleLogsChunk> visible_chunk{};
        std::vector<Fluxion::API::LogsPlugin::Data::ColumnDetails> table_header{};

        Graphite::Common::DataStructures::TCopyLockingDoubleBuffer<Data::Logs::SearchedLog> searched_log{};
    } logs{};
};

namespace DefaultState {

AppState Make();

std::vector<Fluxion::Application::Data::Filters::Tab::Ptr> MakeDefaultTabs();

} // namespace DefaultState

} // namespace Fluxion::Application
