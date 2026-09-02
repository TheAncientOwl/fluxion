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
#include <vector>

#include "Graphite/Logger.hpp"
#include "Scrolls/Papyrus.hpp"

using namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls;

struct PapyrusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Graphite::Logger::DisableAllScopes();

        auto const* info = ::testing::UnitTest::GetInstance()->current_test_info();
        std::string filename =
            std::string("test_") + info->test_suite_name() + "_" + info->name() + ".bin";

        test_path = std::filesystem::temp_directory_path() / filename;
        std::error_code ec;
        std::filesystem::remove(test_path, ec);
    }

    void TearDown() override
    {
        file.Close();

        std::error_code ec;
        std::filesystem::remove(test_path, ec);
    }

    std::filesystem::path test_path;
    Papyrus file;
};

TEST_F(PapyrusTest, OpenWrite)
{
    constexpr std::size_t kCols = 3;
    auto const result = file.OpenWrite(test_path, 512, kCols);

    EXPECT_EQ(result, MappedBinaryStringsFile::EWriteStatus::Success);
    EXPECT_TRUE(file.IsOpen());
    EXPECT_FALSE(file.IsReadOnly());
    EXPECT_EQ(file.GetCapacity(), 512);
    EXPECT_EQ(file.GetSize(), 0);
    EXPECT_EQ(file.GetOffset(), 0);
    EXPECT_EQ(file.GetLinesCount(), 0);
    EXPECT_EQ(file.GetElementsPerLine(), kCols);
    EXPECT_TRUE(std::filesystem::exists(test_path));
}

TEST_F(PapyrusTest, OpenRead)
{
    constexpr std::size_t kCols = 2;
    ASSERT_EQ(file.OpenWrite(test_path, 512, kCols), MappedBinaryStringsFile::EWriteStatus::Success);

    Papyrus::Line const line1 = {"timestamp_1", "log_payload_1"};
    auto const write_res = file.Write(line1);
    ASSERT_EQ(write_res.status, MappedBinaryStringsFile::EWriteStatus::Success);

    // Re-open clean as ReadOnly
    ASSERT_TRUE(file.DowngradeReadOnly());

    EXPECT_TRUE(file.IsOpen());
    EXPECT_TRUE(file.IsReadOnly());
    EXPECT_EQ(file.GetLinesCount(), 1);
    EXPECT_EQ(file.GetElementsPerLine(), kCols);

    Papyrus::Line out_line;
    auto const read_result = file.ReadNext(out_line);
    EXPECT_EQ(read_result.status, MappedBinaryStringsFile::EReadStatus::Success);
    EXPECT_EQ(out_line, line1);
}

TEST_F(PapyrusTest, Close)
{
    constexpr std::size_t kCols = 2;
    ASSERT_EQ(file.OpenWrite(test_path, 512, kCols), MappedBinaryStringsFile::EWriteStatus::Success);
    auto const line{Papyrus::Line{"col0", "col1"}};
    std::ignore = file.Write(line);
    EXPECT_TRUE(file.IsOpen());
    EXPECT_EQ(file.GetLinesCount(), 1);

    file.Close();

    EXPECT_FALSE(file.IsOpen());
    EXPECT_FALSE(file.IsReadOnly());
    EXPECT_EQ(file.GetSize(), 0);
    EXPECT_EQ(file.GetCapacity(), 0);
    EXPECT_EQ(file.GetOffset(), 0);
    EXPECT_EQ(file.GetLinesCount(), 0);
    EXPECT_EQ(file.GetElementsPerLine(), 0);
}

TEST_F(PapyrusTest, WriteOnWriteable)
{
    constexpr std::size_t kCols = 2;
    ASSERT_EQ(file.OpenWrite(test_path, 512, kCols), MappedBinaryStringsFile::EWriteStatus::Success);

    Papyrus::Line const valid_line = {"ERROR", "Out of memory"};
    // Header (4 bytes) + payload size per item: (4 + 5) + (4 + 13) = 26 bytes
    constexpr Bytes expected_written = (sizeof(std::uint32_t) + 5) + (sizeof(std::uint32_t) + 13);

    auto const write_result = file.Write(valid_line);

    EXPECT_EQ(write_result.status, MappedBinaryStringsFile::EWriteStatus::Success);
    EXPECT_EQ(write_result.bytes_written, expected_written);
    EXPECT_EQ(file.GetOffset(), expected_written);
    EXPECT_EQ(file.GetSize(), expected_written);
    EXPECT_EQ(file.GetLinesCount(), 1);

    // Writing a line with column count mismatch must fail
    Papyrus::Line const invalid_line = {"ERROR"};
    auto const invalid_result = file.Write(invalid_line);

    EXPECT_EQ(invalid_result.status, MappedBinaryStringsFile::EWriteStatus::Fail);
    EXPECT_EQ(invalid_result.bytes_written, 0);
    EXPECT_EQ(file.GetLinesCount(), 1);
}

TEST_F(PapyrusTest, WriteOnReadonly)
{
    constexpr std::size_t kCols = 2;
    ASSERT_EQ(file.OpenWrite(test_path, 512, kCols), MappedBinaryStringsFile::EWriteStatus::Success);
    ASSERT_TRUE(file.DowngradeReadOnly());
    EXPECT_TRUE(file.IsReadOnly());

    auto const line{Papyrus::Line{"invalid", "write"}};
    auto const write_result = file.Write(line);

    EXPECT_EQ(write_result.status, MappedBinaryStringsFile::EWriteStatus::Fail);
    EXPECT_EQ(write_result.bytes_written, 0);
}

TEST_F(PapyrusTest, ReadAll)
{
    constexpr std::size_t kCols = 3;
    ASSERT_EQ(file.OpenWrite(test_path, 512, kCols), MappedBinaryStringsFile::EWriteStatus::Success);

    std::vector<Papyrus::Line> const expected_lines = {
        {"10:00:01", "INFO", "App started"},
        {"10:00:02", "", ""},
        {"10:00:03", "WARN", "High memory consumption detected"}};

    for (auto const& line : expected_lines)
    {
        ASSERT_EQ(file.Write(line).status, MappedBinaryStringsFile::EWriteStatus::Success);
    }

    EXPECT_EQ(file.GetLinesCount(), expected_lines.size());

    EXPECT_TRUE(file.SeekToLine(0));

    std::vector<Papyrus::Line> actual_lines;
    Papyrus::Line line_buffer;

    while (file.ReadNext(line_buffer).status == MappedBinaryStringsFile::EReadStatus::Success)
    {
        actual_lines.push_back(line_buffer);
    }

    EXPECT_EQ(actual_lines, expected_lines);
}

TEST_F(PapyrusTest, SeekAndRead)
{
    constexpr std::size_t kCols = 2;
    ASSERT_EQ(file.OpenWrite(test_path, 512, kCols), MappedBinaryStringsFile::EWriteStatus::Success);

    Papyrus::Line const line0 = {"entry_0_col0", "entry_0_col1"};
    Papyrus::Line const line1 = {"entry_1_col0", "entry_1_col1"};
    Papyrus::Line const line2 = {"entry_2_col0", "entry_2_col1"};

    ASSERT_EQ(file.Write(line0).status, MappedBinaryStringsFile::EWriteStatus::Success);
    ASSERT_EQ(file.Write(line1).status, MappedBinaryStringsFile::EWriteStatus::Success);
    ASSERT_EQ(file.Write(line2).status, MappedBinaryStringsFile::EWriteStatus::Success);

    // Seek directly to line index 1 in O(1)
    EXPECT_TRUE(file.SeekToLine(1));

    Papyrus::Line read_buffer;
    {
        auto const read_result = file.ReadNext(read_buffer);
        EXPECT_EQ(read_result.status, MappedBinaryStringsFile::EReadStatus::Success);
        EXPECT_EQ(read_buffer, line1);
    }

    {
        auto const read_result = file.ReadNext(read_buffer);
        EXPECT_EQ(read_result.status, MappedBinaryStringsFile::EReadStatus::Success);
        EXPECT_EQ(read_buffer, line2);
    }
}

TEST_F(PapyrusTest, SeekOverSizeAndRead)
{
    constexpr std::size_t kCols = 2;
    ASSERT_EQ(file.OpenWrite(test_path, 512, kCols), MappedBinaryStringsFile::EWriteStatus::Success);

    auto const line{Papyrus::Line{"valid", "entry"}};
    ASSERT_EQ(file.Write(line).status, MappedBinaryStringsFile::EWriteStatus::Success);

    // Seeking past available line index must fail
    EXPECT_FALSE(file.SeekToLine(1));
    EXPECT_FALSE(file.SeekToLine(100));

    // Ensure valid index still works after failed seeks
    EXPECT_TRUE(file.SeekToLine(0));

    Papyrus::Line line_buffer;
    EXPECT_EQ(file.ReadNext(line_buffer).status, MappedBinaryStringsFile::EReadStatus::Success);
    EXPECT_EQ(line_buffer, line);

    // Next read at end of file returns EOF
    EXPECT_EQ(file.ReadNext(line_buffer).status, MappedBinaryStringsFile::EReadStatus::EOFReached);
}

TEST_F(PapyrusTest, DynamicGrowthOnOverflow)
{
    constexpr std::size_t kCols = 2;
    // Pre-allocate exact size for 1 record: 16 bytes
    constexpr Bytes exact_one_line_capacity = 16;
    ASSERT_EQ(
        file.OpenWrite(test_path, exact_one_line_capacity, kCols),
        MappedBinaryStringsFile::EWriteStatus::Success);

    auto const line1{Papyrus::Line{"a123", "b123"}};
    ASSERT_EQ(file.Write(line1).status, MappedBinaryStringsFile::EWriteStatus::Success);
    EXPECT_EQ(file.GetLinesCount(), 1);

    // Second write exceeds initial capacity (16 bytes),
    // but dynamic resizing should grow the file automatically and succeed!
    auto const line2{Papyrus::Line{"c123", "d123"}};
    auto const res = file.Write(line2);

    EXPECT_EQ(res.status, MappedBinaryStringsFile::EWriteStatus::Success);
    EXPECT_EQ(file.GetLinesCount(), 2);

    // Verify both lines can be read back accurately after growth
    EXPECT_TRUE(file.SeekToLine(0));

    Papyrus::Line read_buffer;
    ASSERT_EQ(file.ReadNext(read_buffer).status, MappedBinaryStringsFile::EReadStatus::Success);
    EXPECT_EQ(read_buffer, line1);

    ASSERT_EQ(file.ReadNext(read_buffer).status, MappedBinaryStringsFile::EReadStatus::Success);
    EXPECT_EQ(read_buffer, line2);
}
