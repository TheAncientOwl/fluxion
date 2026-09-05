/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SQLiteStorageTest.cpp
/// @author Alexandru Delegeanu
/// @version 9.4
/// @brief Logs::Text::RegexTags::V9::SQLiteStorage Google Test Suite
///

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include "SQLite/SQLiteStorage.hpp"

using namespace Fluxion::Plugins::Logs::Text::RegexTags::V9;

class SQLiteStorageTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_database_path =
            std::filesystem::temp_directory_path() / "fluxion_v9_sqlite_storage_test.db";
        std::filesystem::remove(m_database_path);
    }

    void TearDown() override
    {
        m_storage.Close();
        std::filesystem::remove(m_database_path);
    }

    void OpenStorage(std::size_t id_offset = 0)
    {
        ASSERT_TRUE(m_storage.Open(m_database_path, {"field_a", "field_b"}, id_offset));
        ASSERT_TRUE(m_storage.BeginTransaction());
    }

    std::filesystem::path m_database_path{};
    SQLiteStorage m_storage{};
};

TEST_F(SQLiteStorageTest, WritesRowsWithConfiguredIdOffset)
{
    OpenStorage(100);

    std::vector<std::vector<std::string_view>> rows{
        {"first", "one"},
        {"second", "two"},
    };
    std::vector<Data::FilteredLog> filtered_logs;

    ASSERT_TRUE(m_storage.WriteChunk(rows, rows.size(), filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    ASSERT_EQ(filtered_logs.size(), 2);
    EXPECT_EQ(filtered_logs[0].log_id, 100);
    EXPECT_EQ(filtered_logs[1].log_id, 101);
}

TEST_F(SQLiteStorageTest, WritesRowsWithSingleWriterPath)
{
    OpenStorage(100);

    std::vector<std::vector<std::string_view>> rows{
        {"first", "one"},
        {"second", "two"},
    };

    ASSERT_TRUE(m_storage.WriteChunkSingleWriter(rows, rows.size()));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::size_t> ids;
    std::vector<std::string> values;
    ASSERT_TRUE(
        m_storage.ReadRowsViews([&](std::size_t const id, std::vector<std::string_view> const& row) {
            ids.push_back(id);
            values.emplace_back(row.at(0));
            return true;
        }));
    EXPECT_EQ(ids, (std::vector<std::size_t>{100, 101}));
    EXPECT_EQ(values, (std::vector<std::string>{"first", "second"}));
}

TEST_F(SQLiteStorageTest, ReadsRowsFromHalfOpenRanges)
{
    OpenStorage(100);

    std::vector<std::vector<std::string_view>> rows{
        {"first", "one"},
        {"second", "two"},
        {"third", "three"},
    };
    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk(rows, rows.size(), filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::string> second_row;
    std::vector<std::string> third_row;
    std::unordered_map<std::size_t, std::vector<std::string>*> destinations{
        {101, &second_row}, {102, &third_row}};
    ASSERT_TRUE(m_storage.ReadRowsByIDsInto({{.begin = 101, .end = 103}}, destinations));

    EXPECT_EQ(second_row, (std::vector<std::string>{"second", "two"}));
    EXPECT_EQ(third_row, (std::vector<std::string>{"third", "three"}));
}

TEST_F(SQLiteStorageTest, ReadsRowsIntoExistingVectors)
{
    OpenStorage(100);

    std::vector<std::vector<std::string_view>> rows{
        {"first", "one"},
        {"second", "two"},
    };
    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk(rows, rows.size(), filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::string> output{"stale", "data"};
    output.reserve(8);
    auto const* const output_storage = output.data();
    std::unordered_map<std::size_t, std::vector<std::string>*> destinations{{100, &output}};

    ASSERT_TRUE(m_storage.ReadRowsByIDsInto({{.begin = 100, .end = 102}}, destinations));

    EXPECT_EQ(output, (std::vector<std::string>{"first", "one"}));
    EXPECT_EQ(output.data(), output_storage);
}

TEST_F(SQLiteStorageTest, ReadsMultipleRangesAndIgnoresEmptyRanges)
{
    OpenStorage();

    std::vector<std::vector<std::string_view>> rows{
        {"zero", "0"},
        {"one", "1"},
        {"two", "2"},
        {"three", "3"},
    };
    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk(rows, rows.size(), filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::string> one_row;
    std::vector<std::string> three_row;
    std::unordered_map<std::size_t, std::vector<std::string>*> destinations{
        {1, &one_row}, {3, &three_row}};
    ASSERT_TRUE(m_storage.ReadRowsByIDsInto(
        {{.begin = 1, .end = 2}, {.begin = 3, .end = 3}, {.begin = 3, .end = 4}}, destinations));

    EXPECT_EQ(one_row, (std::vector<std::string>{"one", "1"}));
    EXPECT_EQ(three_row, (std::vector<std::string>{"three", "3"}));
}

TEST_F(SQLiteStorageTest, ReadsAllRowsInIdOrder)
{
    OpenStorage(50);

    std::vector<std::vector<std::string_view>> rows{
        {"first", "one"},
        {"second", "two"},
    };
    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk(rows, rows.size(), filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::size_t> ids;
    std::vector<std::string> values;
    ASSERT_TRUE(
        m_storage.ReadRowsViews([&](std::size_t const id, std::vector<std::string_view> const& row) {
            ids.push_back(id);
            values.emplace_back(row.at(0));
            return true;
        }));

    EXPECT_EQ(ids, (std::vector<std::size_t>{50, 51}));
    EXPECT_EQ(values, (std::vector<std::string>{"first", "second"}));
}

TEST_F(SQLiteStorageTest, WritesOnlyActiveRowsFromChunk)
{
    OpenStorage();

    std::vector<std::vector<std::string_view>> rows{
        {"active", "row"},
        {"ignored", "row"},
    };
    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk(rows, 1, filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::string> values;
    ASSERT_TRUE(m_storage.ReadRowsViews([&](std::size_t, std::vector<std::string_view> const& row) {
        values.emplace_back(row.at(0));
        return true;
    }));

    ASSERT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], "active");
    ASSERT_EQ(filtered_logs.size(), 1);
    EXPECT_EQ(filtered_logs[0].log_id, 0);
}

TEST_F(SQLiteStorageTest, MissingIdsAreNotReturned)
{
    OpenStorage();

    std::vector<std::vector<std::string_view>> rows{{"only", "row"}};
    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk(rows, rows.size(), filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::string> output;
    std::unordered_map<std::size_t, std::vector<std::string>*> destinations{{10, &output}};
    ASSERT_TRUE(m_storage.ReadRowsByIDsInto({{.begin = 10, .end = 20}}, destinations));
    EXPECT_TRUE(output.empty());
}

TEST_F(SQLiteStorageTest, ReopenReplacesPreviousDatabaseContents)
{
    OpenStorage();

    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk({{"old", "row"}}, 1, filtered_logs));
    ASSERT_TRUE(m_storage.Commit());
    m_storage.Close();

    std::filesystem::remove(m_database_path);
    ASSERT_TRUE(m_storage.Open(m_database_path, {"field_a", "field_b"}, 200));
    ASSERT_TRUE(m_storage.BeginTransaction());
    ASSERT_TRUE(m_storage.WriteChunk({{"new", "row"}}, 1, filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::size_t> ids;
    std::vector<std::string> values;
    ASSERT_TRUE(
        m_storage.ReadRowsViews([&](std::size_t const id, std::vector<std::string_view> const& row) {
            ids.push_back(id);
            values.emplace_back(row.at(0));
            return true;
        }));
    ASSERT_EQ(ids, (std::vector<std::size_t>{200}));
    EXPECT_EQ(values, (std::vector<std::string>{"new"}));
}

TEST_F(SQLiteStorageTest, StreamsRowsAsStringViews)
{
    OpenStorage(10);

    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk({{"first", "one"}, {"second", "two"}}, 2, filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::size_t> ids;
    std::vector<std::string> values;
    ASSERT_TRUE(
        m_storage.ReadRowsViews([&](std::size_t const id, std::vector<std::string_view> const& row) {
            ids.push_back(id);
            values.emplace_back(row.at(0));
            return true;
        }));

    EXPECT_EQ(ids, (std::vector<std::size_t>{10, 11}));
    EXPECT_EQ(values, (std::vector<std::string>{"first", "second"}));
}

TEST_F(SQLiteStorageTest, StreamingReadCanStopEarly)
{
    OpenStorage();

    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk({{"first", "one"}, {"second", "two"}}, 2, filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::size_t rows_seen{0};
    EXPECT_FALSE(m_storage.ReadRowsViews([&](std::size_t, std::vector<std::string_view> const&) {
        ++rows_seen;
        return false;
    }));
    EXPECT_EQ(rows_seen, 1);
}
