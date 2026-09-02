/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file MappedBinaryStringsFile.cpp
/// @author Alexandru Delegeanu
/// @version 7.0
/// @brief Implementation of @see MappedBinaryStringsFile.hpp
///

#include <algorithm>
#include <cstring>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "Graphite/Logger.hpp"
#include "MappedBinaryStringsFile.hpp"

DEFINE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls::MappedBinaryStringsFile);
USE_LOG_SCOPE(Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls::MappedBinaryStringsFile);

namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls {

using StringLengthType = std::uint32_t;
constexpr Bytes HEADER_SIZE = sizeof(StringLengthType);

void MappedBinaryStringsFile::Deleter::operator()(char* ptr) const noexcept
{
    if (!ptr)
    {
        return;
    }

#if defined(_WIN32)
    ::UnmapViewOfFile(static_cast<LPCVOID>(ptr));
#else
    if (ptr != MAP_FAILED)
    {
        ::munmap(ptr, size);
    }
#endif
}

MappedBinaryStringsFile::EWriteStatus MappedBinaryStringsFile::OpenWrite(
    std::filesystem::path const& path,
    Bytes const max_size)
{
    LOG_SCOPE("::OpenWrite()");
    if (max_size == 0)
    {
        return EWriteStatus::Fail;
    }

    m_data.reset();
    m_path = path;
    m_size = 0;
    m_max_size = max_size;
    m_offset = 0;
    m_is_read_only = false;
    m_is_empty = false;

#if defined(_WIN32)
    HANDLE hFile = ::CreateFileW(
        path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return EWriteStatus::Fail;
    }

    LARGE_INTEGER li;
    li.QuadPart = static_cast<LONGLONG>(max_size);
    if (!::SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN) || !::SetEndOfFile(hFile))
    {
        ::CloseHandle(hFile);
        return EWriteStatus::Fail;
    }

    HANDLE hMapping = ::CreateFileMappingW(hFile, nullptr, PAGE_READWRITE, 0, 0, nullptr);
    ::CloseHandle(hFile);

    if (!hMapping)
    {
        return EWriteStatus::Fail;
    }

    void* ptr = ::MapViewOfFile(hMapping, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
    ::CloseHandle(hMapping);

    if (!ptr)
    {
        return EWriteStatus::Fail;
    }
#else
    int const fd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        return EWriteStatus::Fail;
    }

    if (::ftruncate(fd, static_cast<off_t>(max_size)) == -1)
    {
        ::close(fd);
        return EWriteStatus::Fail;
    }

    void* ptr = ::mmap(nullptr, max_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ::close(fd);

    if (ptr == MAP_FAILED)
    {
        return EWriteStatus::Fail;
    }
#endif

    m_data = std::unique_ptr<char, Deleter>(static_cast<char*>(ptr), Deleter{max_size});
    return EWriteStatus::Success;
}

bool MappedBinaryStringsFile::OpenReadOnly(std::filesystem::path const& path)
{
    LOG_SCOPE("::OpenReadOnly()");

    m_data.reset();
    m_path = path;
    m_offset = 0;
    m_is_read_only = true;

#if defined(_WIN32)
    HANDLE hFile = ::CreateFileW(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    LARGE_INTEGER li;
    if (!::GetFileSizeEx(hFile, &li))
    {
        ::CloseHandle(hFile);
        return false;
    }

    auto const file_size = static_cast<Bytes>(li.QuadPart);
    if (file_size == 0)
    {
        ::CloseHandle(hFile);
        m_size = 0;
        m_max_size = 0;
        m_is_empty = true;
        return true;
    }

    HANDLE hMapping = ::CreateFileMappingW(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    ::CloseHandle(hFile);

    if (!hMapping)
    {
        return false;
    }

    void* ptr = ::MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    ::CloseHandle(hMapping);

    if (!ptr)
    {
        return false;
    }
#else
    int const fd = ::open(path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        return false;
    }

    struct stat sb;
    if (::fstat(fd, &sb) == -1)
    {
        ::close(fd);
        return false;
    }

    auto const file_size = static_cast<Bytes>(sb.st_size);
    if (file_size == 0)
    {
        ::close(fd);
        m_size = 0;
        m_max_size = 0;
        m_is_empty = true;
        return true;
    }

    void* ptr = ::mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
    ::close(fd);

    if (ptr == MAP_FAILED)
    {
        return false;
    }
#endif

    m_size = file_size;
    m_max_size = file_size;
    m_is_empty = false;
    m_data = std::unique_ptr<char, Deleter>(static_cast<char*>(ptr), Deleter{file_size});
    return true;
}

void MappedBinaryStringsFile::Close() noexcept
{
    LOG_SCOPE("::Close()");

    // If we were in write mode and wrote data, trim trailing pre-allocated capacity on disk
    if (!m_is_read_only && m_data && !m_path.empty() && m_size < m_max_size)
    {
        m_data.reset(); // Must unmap before truncating on Windows
#if defined(_WIN32)
        HANDLE hFile = ::CreateFileW(
            m_path.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            LARGE_INTEGER li;
            li.QuadPart = static_cast<LONGLONG>(m_size);
            if (::SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN))
            {
                ::SetEndOfFile(hFile);
            }
            ::CloseHandle(hFile);
        }
#else
        ::truncate(m_path.c_str(), static_cast<off_t>(m_size));
#endif
    }
    else
    {
        m_data.reset();
    }

    m_path.clear();
    m_size = 0;
    m_max_size = 0;
    m_offset = 0;
    m_is_read_only = false;
    m_is_empty = false;
}

bool MappedBinaryStringsFile::DowngradeReadOnly()
{
    LOG_SCOPE("::DowngradeReadOnly()");

    if (!IsOpen() || m_is_read_only)
    {
        return false;
    }

    Bytes const final_size = m_size;

    m_data.reset();

    if (final_size > 0)
    {
#if defined(_WIN32)
        HANDLE hFile = ::CreateFileW(
            m_path.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            LARGE_INTEGER li;
            li.QuadPart = static_cast<LONGLONG>(final_size);
            if (::SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN))
            {
                ::SetEndOfFile(hFile);
            }
            ::CloseHandle(hFile);
        }
#else
        ::truncate(m_path.c_str(), static_cast<off_t>(final_size));
#endif
    }

    return OpenReadOnly(m_path);
}

bool MappedBinaryStringsFile::ResizeFile(Bytes const new_max_size)
{
    LOG_SCOPE("::ResizeFile()");

    if (new_max_size <= m_max_size || m_is_read_only || !m_path.empty() == false)
    {
        return false;
    }

    // 1. Unmap current view (this triggers the Deleter and releases the old mapping)
    m_data.reset();

#if defined(_WIN32)
    HANDLE hFile = ::CreateFileW(
        m_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    LARGE_INTEGER li;
    li.QuadPart = static_cast<LONGLONG>(new_max_size);
    if (!::SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN) || !::SetEndOfFile(hFile))
    {
        ::CloseHandle(hFile);
        return false;
    }

    HANDLE hMapping = ::CreateFileMappingW(hFile, nullptr, PAGE_READWRITE, 0, 0, nullptr);
    ::CloseHandle(hFile);

    if (!hMapping)
    {
        return false;
    }

    void* ptr = ::MapViewOfFile(hMapping, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
    ::CloseHandle(hMapping);

    if (!ptr)
    {
        return false;
    }

    m_max_size = new_max_size;
    m_data = std::unique_ptr<char, Deleter>(static_cast<char*>(ptr), Deleter{new_max_size});
    return true;

#else
    int const fd = ::open(m_path.c_str(), O_RDWR);
    if (fd == -1)
    {
        return false;
    }

    if (::ftruncate(fd, static_cast<off_t>(new_max_size)) == -1)
    {
        ::close(fd);
        return false;
    }

    void* ptr = ::mmap(nullptr, new_max_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ::close(fd);

    if (ptr == MAP_FAILED)
    {
        return false;
    }

    m_max_size = new_max_size;
    m_data = std::unique_ptr<char, Deleter>(static_cast<char*>(ptr), Deleter{new_max_size});
    return true;
#endif
}

MappedBinaryStringsFile::WriteResult MappedBinaryStringsFile::Write(std::string_view const sv)
{
    // LOG_SCOPE("::Write()");
    if (m_is_read_only || !m_data)
    {
        return WriteResult{.status = EWriteStatus::Fail, .bytes_written = 0};
    }

    Bytes const payload_len = sv.size();
    Bytes const total_record_len = HEADER_SIZE + payload_len;

    if (m_offset + total_record_len > m_max_size)
    {
        // 25% growth
        Bytes const percentage_growth = m_max_size / 4;
        Bytes const min_required_growth = total_record_len + 4 * 1024 * 1024; // 4MB safety floor
        Bytes const new_max_size = m_max_size + std::max(percentage_growth, min_required_growth);

        LOG_INFO("::Write(): Trying to resize the file from {} to {}", m_max_size, new_max_size);

        if (!ResizeFile(new_max_size))
        {
            return WriteResult{.status = EWriteStatus::OutOfCapacity, .bytes_written = 0};
        }
        LOG_INFO("::Write(): File resized successfuly!", m_max_size, new_max_size);
    }

    auto const len_header = static_cast<StringLengthType>(payload_len);
    std::memcpy(m_data.get() + m_offset, &len_header, HEADER_SIZE);

    if (payload_len > 0)
    {
        std::memcpy(m_data.get() + m_offset + HEADER_SIZE, sv.data(), payload_len);
    }

    m_offset += total_record_len;
    m_size = std::max(m_size, m_offset);

    return WriteResult{.status = EWriteStatus::Success, .bytes_written = total_record_len};
}

bool MappedBinaryStringsFile::SeekTo(Bytes const offset) noexcept
{
    LOG_SCOPE("::SeekTo()");
    if (offset > m_size)
    {
        return false;
    }
    m_offset = offset;
    return true;
}

bool MappedBinaryStringsFile::HasNext() const noexcept
{
    LOG_SCOPE("::HasNext()");
    return IsOpen() && (m_offset < m_size);
}

MappedBinaryStringsFile::ReadResult MappedBinaryStringsFile::ReadNext() noexcept
{
    LOG_SCOPE("::ReadNext()");
    if (!m_data)
    {
        return ReadResult{.status = EReadStatus::NotOpen, .data = {}};
    }

    if (m_offset >= m_size)
    {
        return ReadResult{.status = EReadStatus::EOFReached, .data = {}};
    }

    // Header truncated mid-read
    if (m_offset + HEADER_SIZE > m_size)
    {
        return ReadResult{.status = EReadStatus::CorruptData, .data = {}};
    }

    StringLengthType payload_len{0};
    std::memcpy(&payload_len, m_data.get() + m_offset, HEADER_SIZE);

    // Payload length pushes past end of file
    if (m_offset + HEADER_SIZE + payload_len > m_size)
    {
        return ReadResult{.status = EReadStatus::CorruptData, .data = {}};
    }

    m_offset += HEADER_SIZE;
    std::string_view const sv(m_data.get() + m_offset, payload_len);
    m_offset += payload_len;

    return ReadResult{.status = EReadStatus::Success, .data = sv};
}

[[nodiscard]] bool MappedBinaryStringsFile::IsOpen() const noexcept
{
    return m_data != nullptr || m_is_empty;
}
[[nodiscard]] bool MappedBinaryStringsFile::IsReadOnly() const noexcept
{
    return m_is_read_only;
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls
