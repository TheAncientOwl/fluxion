/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Papyrus.cpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Implementation of @see Papyrus.hpp
///

#include "Papyrus.hpp"
#include "Graphite/Logger.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls::Papyrus);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls::Papyrus);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls {

MappedBinaryStringsFile::EWriteStatus Papyrus::OpenWrite(
    std::filesystem::path const& path,
    Bytes const max_size,
    std::size_t const elements_per_line)
{
    LOG_SCOPE("::OpenWrite()");
    Close();

    if (elements_per_line == 0)
    {
        return EWriteStatus::Fail;
    }

    m_elements_per_line = elements_per_line;
    return MappedBinaryStringsFile::OpenWrite(path, max_size);
}

bool Papyrus::OpenReadOnly(std::filesystem::path const& path, std::size_t const elements_per_line)
{
    LOG_SCOPE("::OpenReadOnly()");
    Close();

    if (elements_per_line == 0)
    {
        return false;
    }

    m_elements_per_line = elements_per_line;

    if (!MappedBinaryStringsFile::OpenReadOnly(path))
    {
        return false;
    }

    return BuildLineIndex();
}

void Papyrus::Close() noexcept
{
    LOG_SCOPE("::Close()");
    m_lines_offsets.clear();
    m_elements_per_line = 0;
    MappedBinaryStringsFile::Close();
}

bool Papyrus::DowngradeReadOnly()
{
    LOG_SCOPE("::DowngradeReadOnly()");
    if (!MappedBinaryStringsFile::DowngradeReadOnly())
    {
        return false;
    }

    return BuildLineIndex();
}

MappedBinaryStringsFile::WriteResult Papyrus::Write(Line const& line)
{
    // LOG_SCOPE("::Write()");
    if (line.size() != m_elements_per_line || !IsOpen() || IsReadOnly())
    {
        return WriteResult{.status = EWriteStatus::Fail, .bytes_written = 0};
    }

    Bytes const line_start_offset = GetOffset();
    Bytes total_bytes_written = 0;

    for (auto const sv : line)
    {
        auto const res = MappedBinaryStringsFile::Write(sv);

        if (res.status != EWriteStatus::Success)
        {
            // Transactional Rollback: Restore seek pointer to start of line
            std::ignore = MappedBinaryStringsFile::SeekTo(line_start_offset);
            return res;
        }

        total_bytes_written += res.bytes_written;
    }

    m_lines_offsets.push_back(line_start_offset);

    return WriteResult{.status = EWriteStatus::Success, .bytes_written = total_bytes_written};
}

bool Papyrus::SeekToLine(std::size_t const line_index) noexcept
{
    LOG_SCOPE("::SeekToLine()");
    if (line_index >= m_lines_offsets.size())
    {
        return false;
    }

    return MappedBinaryStringsFile::SeekTo(m_lines_offsets[line_index]);
}

MappedBinaryStringsFile::ReadResult Papyrus::ReadNext(Line& out_line) noexcept
{
    LOG_SCOPE("::ReadNext()");
    if (!IsOpen())
    {
        return ReadResult{.status = EReadStatus::NotOpen, .data = {}};
    }

    out_line.clear();
    out_line.reserve(m_elements_per_line);

    Bytes const start_offset = GetOffset();

    for (std::size_t i = 0; i < m_elements_per_line; ++i)
    {
        auto const res = MappedBinaryStringsFile::ReadNext();

        if (res.status != EReadStatus::Success)
        {
            out_line.clear();
            std::ignore = MappedBinaryStringsFile::SeekTo(start_offset); // Revert on read failure
            return res;
        }

        out_line.push_back(res.data);
    }

    return ReadResult{.status = EReadStatus::Success, .data = {}};
}

bool Papyrus::BuildLineIndex()
{
    LOG_SCOPE("::BuildLineIndex()");
    m_lines_offsets.clear();

    if (GetSize() == 0)
    {
        return true;
    }

    std::ignore = MappedBinaryStringsFile::SeekTo(0);

    while (HasNext())
    {
        Bytes const line_start = GetOffset();

        for (std::size_t i = 0; i < m_elements_per_line; ++i)
        {
            auto const res = MappedBinaryStringsFile::ReadNext();
            if (res.status != EReadStatus::Success)
            {
                m_lines_offsets.clear();
                return false; // Corrupted file structure
            }
        }

        m_lines_offsets.push_back(line_start);
    }

    std::ignore = MappedBinaryStringsFile::SeekTo(0);
    return true;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls
