
/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Wrapper.cpp
/// @author Alexandru Delegeanu
/// @version 0.1
/// @brief CSV wrapper implementation for miocsv Reader and Writer
///

#include "Wrapper.hpp"

#include <exception> // IWYU pragma: keep <for miocsv>
#include <memory>

#include "miocsv.h"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V1::CSV {

// ============================================================================
// Reader Implementation
// ============================================================================

class Reader::Impl
{
public:
    Impl(std::string const& file_path, char delimiter)
        : m_reader(file_path, delimiter)
        , m_current_iter(m_reader.begin())
        , m_end_iter(m_reader.end())
        , m_current_row()
    {
        advance();
    }

    bool has_next() const { return m_current_iter != m_end_iter; }

    std::vector<std::string> read_next()
    {
        auto row = std::move(m_current_row);
        ++m_current_iter;
        advance();
        return row;
    }

    std::size_t get_row_num() const { return m_reader.get_row_num(); }

private:
    void advance()
    {
        if (m_current_iter != m_end_iter)
        {
            auto const& row = *m_current_iter;
            m_current_row.clear();
            m_current_row.reserve(row.size());
            for (auto const& field : row)
            {
                m_current_row.push_back(field);
            }
        }
        else
        {
            m_current_row.clear();
        }
    }

    miocsv::MIOReader m_reader;
    miocsv::BaseReader::ReaderIterator m_current_iter;
    miocsv::BaseReader::ReaderIterator m_end_iter;
    std::vector<std::string> m_current_row;

    friend class Reader;
};

// ============================================================================
// Reader::Iterator Implementation
// ============================================================================

Reader::Iterator::Iterator(Reader* reader, bool is_end) : m_reader(reader), m_is_end(is_end)
{
}

Reader::Iterator& Reader::Iterator::operator++()
{
    if (!m_is_end && m_reader)
    {
        m_reader->read_next();
        if (!m_reader->has_next())
        {
            m_is_end = true;
        }
    }
    return *this;
}

bool Reader::Iterator::operator==(Iterator const& other) const
{
    return m_reader == other.m_reader && m_is_end == other.m_is_end;
}

bool Reader::Iterator::operator!=(Iterator const& other) const
{
    return !(*this == other);
}

std::vector<std::string> const& Reader::Iterator::operator*() const
{
    return m_reader->get_current_row();
}

// ============================================================================
// Reader Implementation
// ============================================================================

Reader::Reader(std::string const& file_path, char delimiter)
    : m_impl(std::make_unique<Impl>(file_path, delimiter))
{
}

Reader::~Reader() = default;

Reader::Iterator Reader::begin()
{
    return Iterator{this, false};
}

Reader::Iterator Reader::end()
{
    return Iterator{this, true};
}

bool Reader::has_next() const
{
    return m_impl->has_next();
}

std::vector<std::string> Reader::read_next()
{
    return m_impl->read_next();
}

std::size_t Reader::get_row_num() const
{
    return m_impl->get_row_num();
}

std::vector<std::string> const& Reader::get_current_row() const
{
    return m_impl->m_current_row;
}

// ============================================================================
// Writer Implementation
// ============================================================================

class Writer::Impl
{
public:
    Impl(const std::string& file_path, char delimiter) : m_writer(file_path, delimiter) {}

    void write_row(const std::vector<std::string>& row)
    {
        // Convert std::vector to miocsv::Row
        miocsv::Row csv_row;
        for (const auto& field : row)
        {
            csv_row.append(field);
        }
        m_writer.write_row(csv_row);
    }

private:
    miocsv::Writer m_writer;
};

Writer::Writer(const std::string& file_path, char delimiter)
    : m_impl(std::make_unique<Impl>(file_path, delimiter))
{
}

Writer::~Writer() = default;

void Writer::write_row(const std::vector<std::string>& row)
{
    m_impl->write_row(row);
}

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V1::CSV
