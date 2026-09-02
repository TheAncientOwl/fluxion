/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Scribe.cpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Papyrus scrolls manager implementation
///

#include <algorithm>
#include <format>

#include "Graphite/Logger.hpp"
#include "Scribe.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls::Scribe);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls::Scribe);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls {

// --------------------------------------------------------------------------
// PapyrusWriter Implementation
// --------------------------------------------------------------------------

Scribe::PapyrusWriter::PapyrusWriter(Papyrus* scroll) noexcept : m_scroll(scroll)
{
}

Papyrus::WriteResult Scribe::PapyrusWriter::Write(Papyrus::Line const& line)
{
    GRAPHITE_ASSERT(m_scroll != nullptr, "::Scribe::PapyrusWriter::Write(): using nullptr scroll");
    return m_scroll->Write(line);
}

// --------------------------------------------------------------------------
// Scribe Implementation
// --------------------------------------------------------------------------

Papyrus::EWriteStatus Scribe::OpenWrite(
    std::filesystem::path const& directory_path,
    std::size_t const scrolls_count,
    Bytes const max_size_per_scroll,
    std::size_t const elements_per_line)
{
    LOG_SCOPE("::OpenWrite()");
    Close();

    if (scrolls_count == 0)
    {
        return Papyrus::EWriteStatus::Fail;
    }

    std::error_code ec;
    std::filesystem::create_directories(directory_path, ec);
    if (ec)
    {
        return Papyrus::EWriteStatus::Fail;
    }

    m_scrolls.reserve(scrolls_count);

    for (std::size_t scroll_idx = 0; scroll_idx < scrolls_count; ++scroll_idx)
    {
        auto scroll = std::make_unique<Papyrus>();
        auto const scroll_path = directory_path / std::format("scroll_{}.papyrus", scroll_idx);

        auto const status = scroll->OpenWrite(scroll_path, max_size_per_scroll, elements_per_line);

        if (status != Papyrus::EWriteStatus::Success)
        {
            Close();
            return status;
        }

        m_scrolls.push_back(std::move(scroll));
    }

    RebuildLineIndex();
    return Papyrus::EWriteStatus::Success;
}

bool Scribe::OpenReadOnly(
    std::filesystem::path const& directory_path,
    std::size_t const scrolls_count,
    std::size_t const elements_per_line)
{
    LOG_SCOPE("::OpenReadOnly()");
    Close();

    if (scrolls_count == 0)
    {
        return false;
    }

    m_scrolls.reserve(scrolls_count);

    for (std::size_t scroll_idx = 0; scroll_idx < scrolls_count; ++scroll_idx)
    {
        auto scroll = std::make_unique<Papyrus>();
        auto const scroll_path = directory_path / std::format("scroll_{}.papyrus", scroll_idx);

        if (!scroll->OpenReadOnly(scroll_path, elements_per_line))
        {
            Close();
            return false;
        }

        m_scrolls.push_back(std::move(scroll));
    }

    RebuildLineIndex();
    return true;
}

void Scribe::Close() noexcept
{
    LOG_SCOPE("::Close()");
    for (auto& scroll : m_scrolls)
    {
        if (scroll)
        {
            scroll->Close();
        }
    }
    m_scrolls.clear();
    m_scroll_line_offsets.clear();
}

bool Scribe::DowngradeReadOnly()
{
    LOG_SCOPE("::DowngradeReadOnly()");
    bool all_succeeded = true;

    for (auto& scroll : m_scrolls)
    {
        if (scroll && !scroll->DowngradeReadOnly())
        {
            all_succeeded = false;
        }
    }

    RebuildLineIndex();
    return all_succeeded;
}

std::vector<Scribe::PapyrusWriter> Scribe::GetWriters()
{
    LOG_SCOPE("::GetWriters()");
    std::vector<PapyrusWriter> writers{};
    writers.reserve(m_scrolls.size());

    for (auto const& scroll : m_scrolls)
    {
        writers.emplace_back(scroll.get());
    }

    return writers;
}

void Scribe::RebuildLineIndex()
{
    LOG_SCOPE("::RebuildLineIndex()");
    m_scroll_line_offsets.clear();
    m_scroll_line_offsets.reserve(m_scrolls.size() + 1);

    std::size_t accumulated_lines = 0;
    for (auto const& scroll : m_scrolls)
    {
        m_scroll_line_offsets.push_back(accumulated_lines);
        if (scroll)
        {
            accumulated_lines += scroll->GetLinesCount();
        }
    }
    m_scroll_line_offsets.push_back(accumulated_lines);
}

std::size_t Scribe::GetTotalLinesCount() const noexcept
{
    LOG_SCOPE("::GetTotalLinesCount()");
    if (m_scroll_line_offsets.empty())
    {
        return 0;
    }
    return m_scroll_line_offsets.back();
}

bool Scribe::ReadLine(std::size_t const global_index, Papyrus::Line& out_line) const
{
    LOG_SCOPE("::ReadLine()");
    if (m_scrolls.empty() || m_scroll_line_offsets.size() < 2)
    {
        return false;
    }

    if (global_index >= GetTotalLinesCount())
    {
        return false;
    }

    // Binary search to find scroll index containing global_index
    auto const it =
        std::upper_bound(m_scroll_line_offsets.begin(), m_scroll_line_offsets.end(), global_index);

    std::size_t const scroll_index =
        static_cast<std::size_t>(std::distance(m_scroll_line_offsets.begin(), it) - 1);

    std::size_t const local_line_index = global_index - m_scroll_line_offsets[scroll_index];

    auto& scroll = m_scrolls[scroll_index];
    if (!scroll || !scroll->SeekToLine(local_line_index))
    {
        return false;
    }

    auto const result = scroll->ReadNext(out_line);
    return result.status == Papyrus::EReadStatus::Success;
}

// --------------------------------------------------------------------------
// Scribe::Cursor Implementation
// --------------------------------------------------------------------------

Scribe::Cursor::Cursor(Scribe const* scribe) noexcept : m_scribe(scribe)
{
    if (m_scribe)
    {
        m_total_lines = m_scribe->GetTotalLinesCount();
        m_current_global_idx = 0;
        m_active_scroll_idx = 0;
        m_local_line_idx = 0;

        // Skip leading empty scrolls on initialization
        while (m_active_scroll_idx < m_scribe->m_scrolls.size())
        {
            std::size_t const scroll_lines = m_scribe->m_scroll_line_offsets[m_active_scroll_idx + 1] -
                                             m_scribe->m_scroll_line_offsets[m_active_scroll_idx];
            if (scroll_lines > 0)
            {
                break;
            }
            ++m_active_scroll_idx;
        }
    }
}

bool Scribe::Cursor::HasNext() const noexcept
{
    return m_scribe && (m_current_global_idx < m_total_lines);
}

bool Scribe::ReadLineFromScroll(
    std::size_t const scroll_idx,
    std::size_t const local_line_idx,
    Papyrus::Line& out_line) const
{
    if (scroll_idx >= m_scrolls.size() || !m_scrolls[scroll_idx])
    {
        return false;
    }

    auto* scroll = m_scrolls[scroll_idx].get();

    // 1. O(1) seek to the specific line inside this scroll
    if (!scroll->SeekToLine(local_line_idx))
    {
        return false;
    }

    // 2. Read the line into the output buffer
    auto const read_result = scroll->ReadNext(out_line);

    return read_result.status == Papyrus::EReadStatus::Success;
}

bool Scribe::Cursor::ReadNext(Papyrus::Line& out_line)
{
    if (!HasNext() || m_active_scroll_idx >= m_scribe->m_scrolls.size())
    {
        return false;
    }

    // Direct O(1) read from the active scroll buffer
    bool const success = m_scribe->ReadLineFromScroll(m_active_scroll_idx, m_local_line_idx, out_line);

    ++m_current_global_idx;
    ++m_local_line_idx;

    // Check if we exhausted the current scroll, and roll over to the next one
    std::size_t const current_scroll_total = m_scribe->m_scroll_line_offsets[m_active_scroll_idx + 1] -
                                             m_scribe->m_scroll_line_offsets[m_active_scroll_idx];

    if (m_local_line_idx >= current_scroll_total)
    {
        m_local_line_idx = 0;
        ++m_active_scroll_idx;

        // Skip empty scrolls
        while (m_active_scroll_idx < m_scribe->m_scrolls.size())
        {
            std::size_t const next_scroll_lines =
                m_scribe->m_scroll_line_offsets[m_active_scroll_idx + 1] -
                m_scribe->m_scroll_line_offsets[m_active_scroll_idx];
            if (next_scroll_lines > 0)
            {
                break;
            }
            ++m_active_scroll_idx;
        }
    }

    return success;
}

// --- Scribe Factory Method ---

[[nodiscard]] Scribe::Cursor Scribe::GetCursor()
{
    RebuildLineIndex();
    return Cursor{this};
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls
