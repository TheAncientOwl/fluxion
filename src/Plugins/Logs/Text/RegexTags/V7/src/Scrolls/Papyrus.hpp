/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Papyrus.hpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Logs file management
/// @note Stored using @see MappedBinaryStringsFile.hpp
///       file format  : [line-0][line-1]...[line-n]
///       line format  : [size-0][data-0][size-1][data-1]...[size-k][data-k]
///                      where k is the same for all entries in the file
///

#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

#include "MappedBinaryStringsFile.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls {

class Papyrus final : public MappedBinaryStringsFile
{
public:
    using Line = std::vector<std::string_view>;

public:
    Papyrus() = default;
    ~Papyrus() override = default;

    Papyrus(Papyrus const&) = delete;
    Papyrus& operator=(Papyrus const&) = delete;
    Papyrus(Papyrus&&) noexcept = default;
    Papyrus& operator=(Papyrus&&) noexcept = default;

public:
    [[nodiscard]] EWriteStatus OpenWrite(
        std::filesystem::path const& path,
        Bytes const max_size,
        std::size_t const elements_per_line);

    [[nodiscard]] bool OpenReadOnly(std::filesystem::path const& path, std::size_t elements_per_line);

    void Close() noexcept;

    [[nodiscard]] bool DowngradeReadOnly();

    ///
    /// @brief Writes a full line (vector of string_views) transactionally.
    ///
    [[nodiscard]] WriteResult Write(Line const& line);

    ///
    /// @brief Seeks to the start of line at index @param line_index in O(1) time.
    ///
    [[nodiscard]] bool SeekToLine(std::size_t const line_index) noexcept;

    ///
    /// @brief Reads the next line into @param out_line (reuses vector buffer).
    ///
    [[nodiscard]] ReadResult ReadNext(Line& out_line) noexcept;

    [[nodiscard]] std::size_t GetLinesCount() const noexcept { return m_lines_offsets.size(); }
    [[nodiscard]] std::size_t GetElementsPerLine() const noexcept { return m_elements_per_line; }

private:
    // Hide single string_view methods to enforce line-level operations
    using MappedBinaryStringsFile::ReadNext;
    using MappedBinaryStringsFile::SeekTo;
    using MappedBinaryStringsFile::Write;

private:
    bool BuildLineIndex();

private:
    std::vector<Bytes> m_lines_offsets{};
    std::size_t m_elements_per_line{0};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls
