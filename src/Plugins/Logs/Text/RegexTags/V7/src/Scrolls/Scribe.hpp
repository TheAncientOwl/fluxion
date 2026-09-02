/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Scribe.hpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Papyrus scrolls manager
///

#pragma once

#include <concepts>
#include <filesystem>
#include <memory>
#include <vector>

#include "Papyrus.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls {

template <typename Fn>
concept LineBufferGetter = requires(Fn&& getter, std::size_t index) {
    { getter(index) } -> std::same_as<Papyrus::Line&>;
};

class Scribe
{
public:
    class PapyrusWriter
    {
    public:
        explicit PapyrusWriter(Papyrus* scroll) noexcept;

        [[nodiscard]] Papyrus::WriteResult Write(Papyrus::Line const& line);

    private:
        Papyrus* m_scroll{nullptr};
    };

    class Cursor
    {
    public:
        explicit Cursor(Scribe const* scribe) noexcept;

        [[nodiscard]] bool HasNext() const noexcept;
        bool ReadNext(Papyrus::Line& out_line);

    private:
        Scribe const* m_scribe{nullptr};
        std::size_t m_current_global_idx{0};
        std::size_t m_total_lines{0};
        std::size_t m_active_scroll_idx{0};
        std::size_t m_local_line_idx{0};
    };

    struct Range
    {
        std::size_t begin{0}; // inclusive
        std::size_t end{0}; // exclusive
    };

public:
    [[nodiscard]] Papyrus::EWriteStatus OpenWrite(
        std::filesystem::path const& directory_path,
        std::size_t const scrolls_count,
        Bytes const max_size_per_scroll,
        std::size_t const elements_per_line);

    [[nodiscard]] bool OpenReadOnly(
        std::filesystem::path const& directory_path,
        std::size_t const scrolls_count,
        std::size_t const elements_per_line);

    void Close() noexcept;

    [[nodiscard]] bool DowngradeReadOnly();

    [[nodiscard]] std::vector<PapyrusWriter> GetWriters();

    ///
    /// @brief Rebuilds cumulative line offsets across all scrolls.
    ///
    void RebuildLineIndex();

    template <LineBufferGetter TLineBufferGetter>
    void ReadRanges(std::vector<Range> const& ranges, TLineBufferGetter&& line_buffer_getter);

    [[nodiscard]] std::size_t GetTotalLinesCount() const noexcept;

    [[nodiscard]] Cursor GetCursor();

private:
    [[nodiscard]] bool ReadLine(std::size_t const global_index, Papyrus::Line& out_line) const;
    [[nodiscard]] bool ReadLineFromScroll(
        std::size_t const scroll_idx,
        std::size_t const local_line_idx,
        Papyrus::Line& out_line) const;

private:
    std::vector<std::unique_ptr<Papyrus>> m_scrolls{};
    std::vector<std::size_t> m_scroll_line_offsets{}; // Prefix sum array of global line offsets

    friend class Cursor;
};

// --------------------------------------------------------------------------
// Template Implementations
// --------------------------------------------------------------------------

template <LineBufferGetter TLineBufferGetter>
void Scribe::ReadRanges(std::vector<Range> const& ranges, TLineBufferGetter&& line_buffer_getter)
{
    if (m_scrolls.empty())
    {
        return;
    }

    RebuildLineIndex();

    for (auto const& range : ranges)
    {
        for (std::size_t line_idx = range.begin; line_idx < range.end; ++line_idx)
        {
            Papyrus::Line& line_buffer = line_buffer_getter(line_idx);
            std::ignore = ReadLine(line_idx, line_buffer);
        }
    }
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls
