/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file SQLiteStorage.cpp
/// @author Alexandru Delegeanu
/// @version 8.11
/// @brief Implementation of @see SQLiteStorage.hpp
///

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include "Graphite/Logger.hpp"
#include "SQLite/SQLiteStorage.hpp"

namespace Fluxion::Plugins::Logs::Text::RegexTags::V8 {

namespace {
struct LoggerAutoShutdown
{
    ~LoggerAutoShutdown() { Graphite::Logger::GetLogger().Shutdown(); }
};
static LoggerAutoShutdown logger_auto_shutdown;
} // namespace

namespace {

class Text_RegexTags_V8_SQLiteStorageTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Graphite::Logger::DisableAllScopes();

        m_database_path =
            std::filesystem::temp_directory_path() / "fluxion_v8_sqlite_storage_test.db";
        std::filesystem::remove(m_database_path);
    }

    void TearDown() override
    {
        Graphite::Logger::GetLogger().Shutdown();

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

TEST_F(Text_RegexTags_V8_SQLiteStorageTest, WritesRowsWithConfiguredIdOffset)
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

TEST_F(Text_RegexTags_V8_SQLiteStorageTest, ReadsRowsFromHalfOpenRanges)
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

    std::unordered_map<std::size_t, std::vector<std::string>> read_rows;
    ASSERT_TRUE(m_storage.ReadRowsByIDs({{.begin = 101, .end = 103}}, read_rows));

    ASSERT_EQ(read_rows.size(), 2);
    ASSERT_EQ(read_rows.at(101), (std::vector<std::string>{"second", "two"}));
    ASSERT_EQ(read_rows.at(102), (std::vector<std::string>{"third", "three"}));
    EXPECT_EQ(read_rows.find(100), read_rows.end());
}

TEST_F(Text_RegexTags_V8_SQLiteStorageTest, ReadsMultipleRangesAndIgnoresEmptyRanges)
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

    std::unordered_map<std::size_t, std::vector<std::string>> read_rows;
    ASSERT_TRUE(m_storage.ReadRowsByIDs(
        {{.begin = 1, .end = 2}, {.begin = 3, .end = 3}, {.begin = 3, .end = 4}}, read_rows));

    ASSERT_EQ(read_rows.size(), 2);
    EXPECT_EQ(read_rows.at(1), (std::vector<std::string>{"one", "1"}));
    EXPECT_EQ(read_rows.at(3), (std::vector<std::string>{"three", "3"}));
}

TEST_F(Text_RegexTags_V8_SQLiteStorageTest, ReadsAllRowsInIdOrder)
{
    OpenStorage(50);

    std::vector<std::vector<std::string_view>> rows{
        {"first", "one"},
        {"second", "two"},
    };
    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk(rows, rows.size(), filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::pair<std::size_t, std::vector<std::string>>> read_rows;
    ASSERT_TRUE(m_storage.ReadAll(read_rows));

    ASSERT_EQ(read_rows.size(), 2);
    EXPECT_EQ(read_rows[0].first, 50);
    EXPECT_EQ(read_rows[1].first, 51);
    EXPECT_EQ(read_rows[0].second, (std::vector<std::string>{"first", "one"}));
    EXPECT_EQ(read_rows[1].second, (std::vector<std::string>{"second", "two"}));
}

TEST_F(Text_RegexTags_V8_SQLiteStorageTest, WritesOnlyActiveRowsFromChunk)
{
    OpenStorage();

    std::vector<std::vector<std::string_view>> rows{
        {"active", "row"},
        {"ignored", "row"},
    };
    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk(rows, 1, filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::pair<std::size_t, std::vector<std::string>>> read_rows;
    ASSERT_TRUE(m_storage.ReadAll(read_rows));

    ASSERT_EQ(read_rows.size(), 1);
    EXPECT_EQ(read_rows[0].second, (std::vector<std::string>{"active", "row"}));
    ASSERT_EQ(filtered_logs.size(), 1);
    EXPECT_EQ(filtered_logs[0].log_id, 0);
}

TEST_F(Text_RegexTags_V8_SQLiteStorageTest, MissingIdsAreNotReturned)
{
    OpenStorage();

    std::vector<std::vector<std::string_view>> rows{{"only", "row"}};
    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk(rows, rows.size(), filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::unordered_map<std::size_t, std::vector<std::string>> read_rows;
    ASSERT_TRUE(m_storage.ReadRowsByIDs({{.begin = 10, .end = 20}}, read_rows));
    EXPECT_TRUE(read_rows.empty());
}

TEST_F(Text_RegexTags_V8_SQLiteStorageTest, ReopenReplacesPreviousDatabaseContents)
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

    std::vector<std::pair<std::size_t, std::vector<std::string>>> read_rows;
    ASSERT_TRUE(m_storage.ReadAll(read_rows));
    ASSERT_EQ(read_rows.size(), 1);
    EXPECT_EQ(read_rows[0].first, 200);
    EXPECT_EQ(read_rows[0].second, (std::vector<std::string>{"new", "row"}));
}

TEST_F(Text_RegexTags_V8_SQLiteStorageTest, StreamsRowsWithoutMaterializingTheTable)
{
    OpenStorage(10);

    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk({{"first", "one"}, {"second", "two"}}, 2, filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::vector<std::size_t> ids;
    std::vector<std::string> values;
    ASSERT_TRUE(m_storage.ReadRows([&](std::size_t const id, std::vector<std::string> const& row) {
        ids.push_back(id);
        values.push_back(row.at(0));
        return true;
    }));

    EXPECT_EQ(ids, (std::vector<std::size_t>{10, 11}));
    EXPECT_EQ(values, (std::vector<std::string>{"first", "second"}));
}

TEST_F(Text_RegexTags_V8_SQLiteStorageTest, StreamingReadCanStopEarly)
{
    OpenStorage();

    std::vector<Data::FilteredLog> filtered_logs;
    ASSERT_TRUE(m_storage.WriteChunk({{"first", "one"}, {"second", "two"}}, 2, filtered_logs));
    ASSERT_TRUE(m_storage.Commit());

    std::size_t rows_seen{0};
    EXPECT_FALSE(m_storage.ReadRows([&](std::size_t, std::vector<std::string> const&) {
        ++rows_seen;
        return false;
    }));
    EXPECT_EQ(rows_seen, 1);
}

} // namespace
} // namespace Fluxion::Plugins::Logs::Text::RegexTags::V8
