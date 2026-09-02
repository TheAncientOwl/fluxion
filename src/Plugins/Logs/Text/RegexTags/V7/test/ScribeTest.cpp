/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ScrollsTest.cpp
/// @author Alexandru Delegeanu
/// @version 7.1
/// @brief Logs::Text::RegexTags::V7::Scrolls unit tests
///

#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "Graphite/Logger.hpp"
#include "Scrolls/Scribe.hpp"

using namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls;

struct ScribeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Graphite::Logger::DisableAllScopes();

        auto const* info = ::testing::UnitTest::GetInstance()->current_test_info();
        std::string const dir_name =
            std::string("test_dir_") + info->test_suite_name() + "_" + info->name();

        test_path = std::filesystem::temp_directory_path() / dir_name;

        std::error_code ec;
        std::filesystem::remove_all(test_path, ec);
    }

    void TearDown() override
    {
        scribe.Close();

        std::error_code ec;
        std::filesystem::remove_all(test_path, ec);
    }

    std::filesystem::path test_path;
    Scribe scribe;
};

TEST_F(ScribeTest, OpenWrite)
{
    std::size_t const scrolls_count = 3;
    std::size_t const elements_per_line = 2;
    Bytes const max_size = 1024;

    auto const status = scribe.OpenWrite(test_path, scrolls_count, max_size, elements_per_line);

    EXPECT_EQ(status, Papyrus::EWriteStatus::Success);
    EXPECT_TRUE(std::filesystem::exists(test_path));
    EXPECT_EQ(scribe.GetWriters().size(), scrolls_count);
    EXPECT_EQ(scribe.GetTotalLinesCount(), 0);
}

TEST_F(ScribeTest, OpenWriteInvalidZeroScrolls)
{
    auto const status = scribe.OpenWrite(test_path, 0, 1024, 2);

    EXPECT_EQ(status, Papyrus::EWriteStatus::Fail);
    EXPECT_EQ(scribe.GetWriters().size(), 0);
}

TEST_F(ScribeTest, OpenReadOnly)
{
    std::size_t const scrolls_count = 2;
    std::size_t const elements_per_line = 2;
    Bytes const max_size = 1024;

    // Write initial data across scrolls
    ASSERT_EQ(
        scribe.OpenWrite(test_path, scrolls_count, max_size, elements_per_line),
        Papyrus::EWriteStatus::Success);

    auto writers = scribe.GetWriters();
    ASSERT_EQ(writers.size(), 2);

    auto const line1 = Papyrus::Line{"tag_a", "val_a"};
    auto const line2 = Papyrus::Line{"tag_b", "val_b"};

    ASSERT_EQ(writers[0].Write(line1).status, Papyrus::EWriteStatus::Success);
    ASSERT_EQ(writers[1].Write(line2).status, Papyrus::EWriteStatus::Success);

    scribe.Close();

    // Re-open in read-only mode
    EXPECT_TRUE(scribe.OpenReadOnly(test_path, scrolls_count, elements_per_line));
    EXPECT_EQ(scribe.GetTotalLinesCount(), 2);
}

TEST_F(ScribeTest, Close)
{
    ASSERT_EQ(scribe.OpenWrite(test_path, 2, 1024, 2), Papyrus::EWriteStatus::Success);
    EXPECT_EQ(scribe.GetWriters().size(), 2);

    scribe.Close();

    EXPECT_EQ(scribe.GetWriters().size(), 0);
    EXPECT_EQ(scribe.GetTotalLinesCount(), 0);
}

TEST_F(ScribeTest, WriteOnReadOnlyFails)
{
    ASSERT_EQ(scribe.OpenWrite(test_path, 2, 1024, 2), Papyrus::EWriteStatus::Success);

    auto writers = scribe.GetWriters();
    ASSERT_TRUE(scribe.DowngradeReadOnly());

    auto const line = Papyrus::Line{"readonly_key", "readonly_val"};
    auto const write_res = writers[0].Write(line);

    EXPECT_EQ(write_res.status, Papyrus::EWriteStatus::Fail);
    EXPECT_EQ(write_res.bytes_written, 0);
}

TEST_F(ScribeTest, ReadRangesCrossScrolls)
{
    std::size_t const scrolls_count = 2;
    std::size_t const elements_per_line = 2;
    Bytes const max_size = 4096;

    ASSERT_EQ(
        scribe.OpenWrite(test_path, scrolls_count, max_size, elements_per_line),
        Papyrus::EWriteStatus::Success);

    auto writers = scribe.GetWriters();
    ASSERT_EQ(writers.size(), 2);

    // Scroll 0 gets 3 lines
    auto const l0 = Papyrus::Line{"s0_line0", "val0"};
    auto const l1 = Papyrus::Line{"s0_line1", "val1"};
    auto const l2 = Papyrus::Line{"s0_line2", "val2"};

    // Scroll 1 gets 2 lines
    auto const l3 = Papyrus::Line{"s1_line0", "val3"};
    auto const l4 = Papyrus::Line{"s1_line1", "val4"};

    ASSERT_EQ(writers[0].Write(l0).status, Papyrus::EWriteStatus::Success);
    ASSERT_EQ(writers[0].Write(l1).status, Papyrus::EWriteStatus::Success);
    ASSERT_EQ(writers[0].Write(l2).status, Papyrus::EWriteStatus::Success);

    ASSERT_EQ(writers[1].Write(l3).status, Papyrus::EWriteStatus::Success);
    ASSERT_EQ(writers[1].Write(l4).status, Papyrus::EWriteStatus::Success);

    scribe.RebuildLineIndex();
    EXPECT_EQ(scribe.GetTotalLinesCount(), 5);

    // Global mapping:
    // [0..2] -> Scroll 0 (indices 0, 1, 2)
    // [3..4] -> Scroll 1 (indices 0, 1)
    std::vector<Scribe::Range> const ranges = {
        {.begin = 1, .end = 4} // Global line indices 1, 2 (Scroll 0) and 3 (Scroll 1)
    };

    std::unordered_map<std::size_t, Papyrus::Line> buffer_pool{};

    scribe.ReadRanges(ranges, [&buffer_pool](std::size_t const index) -> Papyrus::Line& {
        return buffer_pool[index];
    });

    EXPECT_EQ(buffer_pool.size(), 3);

    // Line at global index 1 (Scroll 0, local index 1)
    ASSERT_EQ(buffer_pool[1].size(), 2);
    EXPECT_EQ(buffer_pool[1][0], "s0_line1");
    EXPECT_EQ(buffer_pool[1][1], "val1");

    // Line at global index 2 (Scroll 0, local index 2)
    ASSERT_EQ(buffer_pool[2].size(), 2);
    EXPECT_EQ(buffer_pool[2][0], "s0_line2");
    EXPECT_EQ(buffer_pool[2][1], "val2");

    // Line at global index 3 (Scroll 1, local index 0)
    ASSERT_EQ(buffer_pool[3].size(), 2);
    EXPECT_EQ(buffer_pool[3][0], "s1_line0");
    EXPECT_EQ(buffer_pool[3][1], "val3");
}

TEST_F(ScribeTest, ReadRangesOutOfBoundIsHandled)
{
    ASSERT_EQ(scribe.OpenWrite(test_path, 1, 1024, 2), Papyrus::EWriteStatus::Success);

    auto writers = scribe.GetWriters();
    auto const line = Papyrus::Line{"valid_key", "valid_val"};
    ASSERT_EQ(writers[0].Write(line).status, Papyrus::EWriteStatus::Success);

    std::vector<Scribe::Range> const ranges = {
        {.begin = 0, .end = 5} // Range exceeds global lines count (1)
    };

    std::unordered_map<std::size_t, Papyrus::Line> buffer_pool;

    scribe.ReadRanges(ranges, [&buffer_pool](std::size_t const index) -> Papyrus::Line& {
        return buffer_pool[index];
    });

    // Only index 0 should be read successfully
    EXPECT_EQ(buffer_pool.size(), 5); // Getter is invoked for indices 0..4
    EXPECT_EQ(buffer_pool[0].size(), 2);
    EXPECT_TRUE(buffer_pool[1].empty());
}

TEST_F(ScribeTest, CursorSequentialReadCrossScrolls)
{
    std::size_t const scrolls_count = 2;
    std::size_t const elements_per_line = 2;
    Bytes const max_size = 4096;

    ASSERT_EQ(
        scribe.OpenWrite(test_path, scrolls_count, max_size, elements_per_line),
        Papyrus::EWriteStatus::Success);

    auto writers = scribe.GetWriters();
    ASSERT_EQ(writers.size(), 2);

    // Scroll 0 gets 2 lines
    auto const l0 = Papyrus::Line{"s0_line0", "val0"};
    auto const l1 = Papyrus::Line{"s0_line1", "val1"};
    // Scroll 1 gets 2 lines
    auto const l2 = Papyrus::Line{"s1_line0", "val2"};
    auto const l3 = Papyrus::Line{"s1_line1", "val3"};

    ASSERT_EQ(writers[0].Write(l0).status, Papyrus::EWriteStatus::Success);
    ASSERT_EQ(writers[0].Write(l1).status, Papyrus::EWriteStatus::Success);
    ASSERT_EQ(writers[1].Write(l2).status, Papyrus::EWriteStatus::Success);
    ASSERT_EQ(writers[1].Write(l3).status, Papyrus::EWriteStatus::Success);

    ASSERT_TRUE(scribe.DowngradeReadOnly());

    auto cursor = scribe.GetCursor();
    std::vector<Papyrus::Line> read_lines;

    while (cursor.HasNext())
    {
        Papyrus::Line line;
        ASSERT_TRUE(cursor.ReadNext(line));
        read_lines.push_back(line);
    }

    ASSERT_EQ(read_lines.size(), 4);
    EXPECT_EQ(read_lines[0], l0);
    EXPECT_EQ(read_lines[1], l1);
    EXPECT_EQ(read_lines[2], l2);
    EXPECT_EQ(read_lines[3], l3);
}
