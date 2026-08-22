
/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file Wrapper.hpp
/// @author Alexandru Delegeanu
/// @version 2.0
/// @brief CSV wrapper for miocsv Reader and Writer
///

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Fluxion::Plugins::Logs::Text::RegexTags::V2::CSV {

class Reader
{
public:
    class Iterator
    {
    public:
        Iterator() = delete;
        explicit Iterator(Reader* reader, bool is_end = false);

        Iterator& operator++();
        bool operator==(Iterator const& other) const;
        bool operator!=(Iterator const& other) const;
        std::vector<std::string> const& operator*() const;

    private:
        Reader* m_reader;
        bool m_is_end;
    };

    Reader() = delete;
    explicit Reader(std::string const& file_path, char delimiter = ',');
    ~Reader();

    Reader(Reader const&) = delete;
    Reader& operator=(Reader const&) = delete;

    Reader(Reader&&) = delete;
    Reader& operator=(Reader&&) = delete;

    /// @brief Get iterator to the beginning
    Iterator begin();

    /// @brief Get iterator to the end
    Iterator end();

    /// @brief Check if there are more rows to read
    bool has_next() const;

    /// @brief Read the next row
    /// @return Vector of strings representing the row
    std::vector<std::string> read_next();

    /// @brief Get the current row number (1-based)
    std::size_t get_row_num() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
    friend class Iterator;

    /// @brief Get the current row (for iterator use)
    std::vector<std::string> const& get_current_row() const;
};

class Writer
{
public:
    Writer() = delete;
    explicit Writer(std::string const& file_path, char delimiter = ',');
    ~Writer();

    Writer(Writer const&) = delete;
    Writer& operator=(Writer const&) = delete;

    Writer(Writer&&) = default;
    Writer& operator=(Writer&&) = delete;

    /// @brief Write a row to the file
    /// @param row Vector of strings representing the row
    void write_row(std::vector<std::string> const& row);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V2::CSV
