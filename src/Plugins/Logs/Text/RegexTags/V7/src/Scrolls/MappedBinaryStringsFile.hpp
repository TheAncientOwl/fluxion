/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MappedBinaryStringsFile.hpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Class responsible for file mmapped IO of string_view's.
/// @note [size-0][data-0][size-1][data-1]...[size-n][data-n]
///

#pragma once

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string_view>

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls {

using Bytes = std::size_t;

class MappedBinaryStringsFile
{
public:
    struct Deleter
    {
        std::size_t size{0};
        void operator()(char* ptr) const noexcept;
    };

    enum class EWriteStatus : std::uint8_t
    {
        None,
        Fail,
        Success,
        OutOfCapacity
    };

    struct WriteResult
    {
        EWriteStatus status{EWriteStatus::None};
        Bytes bytes_written{0};
    };

    enum class EReadStatus : std::uint8_t
    {
        Success,
        EOFReached,
        CorruptData,
        NotOpen
    };

    struct ReadResult
    {
        EReadStatus status{EReadStatus::NotOpen};
        std::string_view data{};
    };

public:
    MappedBinaryStringsFile() = default;
    virtual ~MappedBinaryStringsFile() = default;

    MappedBinaryStringsFile(MappedBinaryStringsFile const&) = delete;
    MappedBinaryStringsFile& operator=(MappedBinaryStringsFile const&) = delete;
    MappedBinaryStringsFile(MappedBinaryStringsFile&&) noexcept = default;
    MappedBinaryStringsFile& operator=(MappedBinaryStringsFile&&) noexcept = default;

public:
    [[nodiscard]] EWriteStatus OpenWrite(std::filesystem::path const& path, Bytes const max_size);
    [[nodiscard]] bool OpenReadOnly(std::filesystem::path const& path);
    ///
    /// @brief Unmaps memory and resets file state
    ///
    void Close() noexcept;

    [[nodiscard]] bool IsOpen() const noexcept;
    [[nodiscard]] bool IsReadOnly() const noexcept;

    [[nodiscard]] bool DowngradeReadOnly();

    ///
    /// @return WriteResult containing status and exact bytes written (HEADER_SIZE + payload)
    ///
    [[nodiscard]] WriteResult Write(std::string_view const sv);

    [[nodiscard]] bool SeekTo(Bytes const offset) noexcept;
    [[nodiscard]] bool HasNext() const noexcept;
    [[nodiscard]] ReadResult ReadNext() noexcept;

    [[nodiscard]] Bytes GetSize() const noexcept { return m_size; }
    [[nodiscard]] Bytes GetCapacity() const noexcept { return m_max_size; }
    [[nodiscard]] Bytes GetOffset() const noexcept { return m_offset; }

private:
    [[nodiscard]] bool ResizeFile(Bytes const max_size);

private:
    std::unique_ptr<char, Deleter> m_data{nullptr, Deleter{}};
    std::filesystem::path m_path{};
    Bytes m_size{0};
    Bytes m_max_size{0};
    Bytes m_offset{0};
    bool m_is_read_only{false};
    bool m_is_empty{false};
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls
