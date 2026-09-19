/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsPluginAPI.hpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Plugin interface of Fluxion logs logic (parse/select/filter/...).
///

#pragma once

#include "Fluxion/API/LogsPlugin/Private/ABI/Safe.hpp"

namespace Fluxion::API::LogsPlugin::Private {

struct LogsPluginAPI
{
    void* instance;

    void (*Destroy)(void* instance);

    ABI::Safe::StringView (*GetDisplayName)(void const* instance);
    ABI::Safe::StringView (*GetDirectoryName)(void const* instance);

    void (*OnEnable)(void* instance, ABI::Safe::OnEnableData const data);
    void (*OnDisable)(void* instance, ABI::Safe::OnDisableData const data);

    void (*RenderMenu)(void* instance);

    void (*ImportLogs)(void* instance, ABI::Safe::StringView const path);

    bool (*GetNextLog)(
        void* instance,
        ABI::Safe::UniqueID const filter_id,
        std::size_t const current_index,
        std::size_t* const out_index);

    bool (*GetPrevLog)(
        void* instance,
        ABI::Safe::UniqueID const filter_id,
        std::size_t const current_index,
        std::size_t* const out_index);

    void (*ApplyFilters)(
        void* instance,
        ABI::Safe::FiltersSpan const filters,
        ABI::Safe::FiltersSpan const highlight_only);

    void (*DisableFilters)(void* instance);

    ABI::Safe::ColumnsDetailsSpan (*GetTableHeader)(void const* instance);

    std::size_t (*GetTotalLogs)(void const* instance);

    void (*GetLogs)(void* instance, ABI::Safe::RangesSpan const ranges, ABI::Safe::LogRowWriter writer);

    std::size_t (*GetLogsOperationTarget)(void const* instance);
    ABI::Safe::ELogsOperationUnit (*GetLogsOperationUnit)(void const* instance);
    std::size_t (*GetLogsOperationProgress)(void const* instance);
};

} // namespace Fluxion::API::LogsPlugin::Private
