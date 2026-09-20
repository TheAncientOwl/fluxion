/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file LogsPluginAdapter.cpp
/// @author Alexandru Delegeanu
/// @version 1.0
/// @brief Implementation of @see LogsPluginAdapter.hpp
///

#include "Fluxion/API/LogsPlugin/Private/LogsPluginAdapter.hpp"
#include "Fluxion/API/LogsPlugin/Private/ABI/Adapter.hpp"
#include "Fluxion/API/LogsPlugin/Private/LogsPluginAPI.hpp"

namespace Fluxion::API::LogsPlugin::Private {

LogsPluginAdapter::LogsPluginAdapter(LogsPluginAPI const api) : m_api{api}
{
}

LogsPluginAdapter::~LogsPluginAdapter()
{
    if (m_api.Destroy && m_api.instance)
    {
        m_api.Destroy(m_api.instance);
    }
}

LogsPluginAdapter& LogsPluginAdapter::operator=(LogsPluginAdapter&& other) noexcept
{
    if (this == &other)
        return *this;

    if (m_api.Destroy && m_api.instance)
    {
        m_api.Destroy(m_api.instance);
    }

    m_api = other.m_api;
    other.m_api = {};

    m_table_header = std::move(other.m_table_header);

    return *this;
}

LogsPluginAdapter::LogsPluginAdapter(LogsPluginAdapter&& other) noexcept
    : m_api{other.m_api}, m_table_header{std::move(other.m_table_header)}
{
    other.m_api = {};
}

std::string_view LogsPluginAdapter::GetDisplayName() const
{
    return ABI::Adapter::ToNative(m_api.GetDisplayName(m_api.instance));
}

std::string_view LogsPluginAdapter::GetDirectoryName() const
{
    return ABI::Adapter::ToNative(m_api.GetDirectoryName(m_api.instance));
}

void LogsPluginAdapter::OnEnable(ABI::Unsafe::OnEnableData const& data)
{
    auto const path = data.plugin_home_path.u8string();

    auto const safe_data = ABI::Safe::OnEnableData{
        .plugin_home_path = ABI::Safe::StringView{
            .data = reinterpret_cast<char const*>(path.data()), .size = path.size()}};

    m_api.OnEnable(m_api.instance, safe_data);
}

void LogsPluginAdapter::OnDisable(ABI::Unsafe::OnDisableData const& data)
{
    m_api.OnDisable(m_api.instance, ABI::Adapter::ToSafe(data));
}

void LogsPluginAdapter::RenderMenu()
{
    m_api.RenderMenu(m_api.instance);
}

void LogsPluginAdapter::ImportLogs(std::filesystem::path const& path)
{
    auto const path_string = path.u8string();

    m_api.ImportLogs(
        m_api.instance,
        ABI::Safe::StringView{
            .data = reinterpret_cast<char const*>(path_string.data()), .size = path_string.size()});
}

std::optional<std::size_t> LogsPluginAdapter::GetNextLog(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index)
{
    auto const safe_filter_id = ABI::Adapter::ToSafe(filter_id);

    std::size_t index{};

    if (!m_api.GetNextLog(m_api.instance, safe_filter_id, current_index, &index))
    {
        return std::nullopt;
    }

    return index;
}

std::optional<std::size_t> LogsPluginAdapter::GetPrevLog(
    Graphite::Common::Utility::UniqueID const& filter_id,
    std::size_t const current_index)
{
    auto const safe_filter_id = ABI::Adapter::ToSafe(filter_id);

    std::size_t index{};

    if (!m_api.GetPrevLog(m_api.instance, safe_filter_id, current_index, &index))
    {
        return std::nullopt;
    }

    return index;
}

void LogsPluginAdapter::ApplyFilters(
    std::span<ABI::Unsafe::Filter const> const filters,
    std::span<ABI::Unsafe::Filter const> const highlight_only)
{
    // >> filters
    std::vector<std::vector<ABI::Safe::Condition>> safe_filter_conditions{};
    // TODO: reserve

    std::vector<ABI::Safe::Filter> safe_filters{};
    safe_filters.reserve(filters.size());

    for (auto const& filter : filters)
    {
        safe_filter_conditions.emplace_back();
        auto& conditions = safe_filter_conditions.back();
        conditions.reserve(filter.conditions.size());

        for (auto const& condition : filter.conditions)
        {
            conditions.emplace_back(ABI::Adapter::ToSafe(condition));
        }

        safe_filters.emplace_back(
            ABI::Safe::Filter{
                .id = ABI::Adapter::ToSafe(filter.id),
                .conditions =
                    ABI::Safe::ConditionsSpan{.data = conditions.data(), .size = conditions.size()},
                .priority = filter.priority,
                .highlight_only = static_cast<std::uint8_t>(filter.highlight_only)});
    }

    // >> highlight_only
    std::vector<std::vector<ABI::Safe::Condition>> safe_highlight_only_conditions{};
    // TODO: reserve

    std::vector<ABI::Safe::Filter> safe_highlight_only;
    safe_highlight_only.reserve(highlight_only.size());

    for (auto const& filter : highlight_only)
    {
        safe_highlight_only_conditions.emplace_back();
        auto& conditions = safe_highlight_only_conditions.back();
        conditions.reserve(filter.conditions.size());

        for (auto const& condition : filter.conditions)
        {
            conditions.emplace_back(ABI::Adapter::ToSafe(condition));
        }

        safe_highlight_only.emplace_back(
            ABI::Safe::Filter{
                .id = ABI::Adapter::ToSafe(filter.id),
                .conditions =
                    ABI::Safe::ConditionsSpan{.data = conditions.data(), .size = conditions.size()},
                .priority = filter.priority,
                .highlight_only = static_cast<std::uint8_t>(filter.highlight_only)});
    }

    // >> call
    m_api.ApplyFilters(
        m_api.instance,
        ABI::Safe::FiltersSpan{.data = safe_filters.data(), .size = safe_filters.size()},
        ABI::Safe::FiltersSpan{.data = safe_highlight_only.data(), .size = safe_highlight_only.size()});
}

void LogsPluginAdapter::DisableFilters()
{
    m_api.DisableFilters(m_api.instance);
}

std::span<Fluxion::API::LogsPlugin::ColumnDetails const> LogsPluginAdapter::GetTableHeader() const
{
    auto const columns = m_api.GetTableHeader(m_api.instance);

    m_table_header.clear();
    m_table_header.reserve(columns.size);

    for (auto const& column : std::span{columns.data, columns.size})
    {
        m_table_header.emplace_back(ABI::Adapter::ToNative(column));
    }

    return m_table_header;
}

std::size_t LogsPluginAdapter::GetTotalLogs() const
{
    return m_api.GetTotalLogs(m_api.instance);
}

void LogsPluginAdapter::GetLogs(
    std::span<ABI::Safe::Range const> const ranges,
    Fluxion::API::LogsPlugin::WriteLogRowDataFn write_data,
    Fluxion::API::LogsPlugin::WriteLogRowMetadataFn write_metadata,
    void* user_data)
{
    m_api.GetLogs(
        m_api.instance,
        ABI::Safe::RangesSpan{.data = ranges.data(), .size = ranges.size()},
        ABI::Safe::LogRowWriter{
            .user_data = user_data, .write_data = write_data, .write_metadata = write_metadata});
}

std::size_t LogsPluginAdapter::GetLogsOperationTarget() const
{
    return m_api.GetLogsOperationTarget(m_api.instance);
}

ABI::Safe::ELogsOperationUnit LogsPluginAdapter::GetLogsOperationUnit() const
{
    return m_api.GetLogsOperationUnit(m_api.instance);
}

std::size_t LogsPluginAdapter::GetLogsOperationProgress() const
{
    return m_api.GetLogsOperationProgress(m_api.instance);
}

} // namespace Fluxion::API::LogsPlugin::Private
