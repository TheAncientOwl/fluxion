/// --------------------------------------------------------------------------
///                     Copyright (c) by Fluxion 2026
/// --------------------------------------------------------------------------
/// @license https://github.com/TheAncientOwl/fluxion/blob/main/LICENSE
///
/// @file ScrollsTest.cpp
/// @author Alexandru Delegeanu
/// @version 7.2
/// @brief Logs::Text::RegexTags::V7::Scrolls unit tests
///

#include <filesystem>
#include <gtest/gtest.h>
#include <string>

#include "Graphite/Logger.hpp"
#include "Scrolls/MappedBinaryStringsFile.hpp"

using namespace Fluxion::Plugins::Logs::Text::RegexTags::V7::Scrolls;

namespace {
struct LoggerAutoShutdown
{
    ~LoggerAutoShutdown() { Graphite::Logger::GetLogger().Shutdown(); }
};
static LoggerAutoShutdown logger_auto_shutdown;
} // namespace

struct Text_RegexTags_V7_WriteableMappedFileTest : public ::testing::Test
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
        Graphite::Logger::GetLogger().Shutdown();

        file.Close();

        std::error_code ec;
        std::filesystem::remove(test_path, ec);
    }

    std::filesystem::path test_path;
    MappedBinaryStringsFile file;
};

TEST_F(Text_RegexTags_V7_WriteableMappedFileTest, OpenWrite)
{
    auto const result = file.OpenWrite(test_path, 512);

    EXPECT_EQ(result, MappedBinaryStringsFile::EWriteStatus::Success);
    EXPECT_TRUE(file.IsOpen());
    EXPECT_FALSE(file.IsReadOnly());
    EXPECT_EQ(file.GetCapacity(), 512);
    EXPECT_EQ(file.GetSize(), 0);
    EXPECT_EQ(file.GetOffset(), 0);
    EXPECT_TRUE(std::filesystem::exists(test_path));
}

TEST_F(Text_RegexTags_V7_WriteableMappedFileTest, OpenRead)
{
    // Write a record and close
    ASSERT_EQ(file.OpenWrite(test_path, 512), MappedBinaryStringsFile::EWriteStatus::Success);
    auto const write_res = file.Write("initial_payload");
    ASSERT_EQ(write_res.status, MappedBinaryStringsFile::EWriteStatus::Success);
    file.Close();

    // Re-open clean as ReadOnly
    EXPECT_TRUE(file.OpenReadOnly(test_path));
    EXPECT_TRUE(file.IsOpen());
    EXPECT_TRUE(file.IsReadOnly());
    EXPECT_EQ(file.GetSize(), write_res.bytes_written);
    EXPECT_EQ(file.GetOffset(), 0);
    auto const read_result = file.ReadNext();
    EXPECT_EQ(read_result.status, MappedBinaryStringsFile::EReadStatus::Success);
    EXPECT_EQ(read_result.data, "initial_payload");
}

TEST_F(Text_RegexTags_V7_WriteableMappedFileTest, Close)
{
    ASSERT_EQ(file.OpenWrite(test_path, 512), MappedBinaryStringsFile::EWriteStatus::Success);
    EXPECT_TRUE(file.IsOpen());

    file.Close();

    EXPECT_FALSE(file.IsOpen());
    EXPECT_FALSE(file.IsReadOnly());
    EXPECT_EQ(file.GetSize(), 0);
    EXPECT_EQ(file.GetCapacity(), 0);
    EXPECT_EQ(file.GetOffset(), 0);
}

TEST_F(Text_RegexTags_V7_WriteableMappedFileTest, WriteOnWriteable)
{
    ASSERT_EQ(file.OpenWrite(test_path, 512), MappedBinaryStringsFile::EWriteStatus::Success);

    std::string_view const payload = "Hello, World!";
    Bytes const expected_written = sizeof(std::uint32_t) + payload.size();

    auto const write_result = file.Write(payload);

    EXPECT_EQ(write_result.status, MappedBinaryStringsFile::EWriteStatus::Success);
    EXPECT_EQ(write_result.bytes_written, expected_written);
    EXPECT_EQ(file.GetOffset(), expected_written);
    EXPECT_EQ(file.GetSize(), expected_written);
}

TEST_F(Text_RegexTags_V7_WriteableMappedFileTest, WriteOnReadonly)
{
    ASSERT_EQ(file.OpenWrite(test_path, 512), MappedBinaryStringsFile::EWriteStatus::Success);
    ASSERT_TRUE(file.DowngradeReadOnly());
    EXPECT_TRUE(file.IsReadOnly());

    auto const write_result = file.Write("invalid_write");

    EXPECT_EQ(write_result.status, MappedBinaryStringsFile::EWriteStatus::Fail);
    EXPECT_EQ(write_result.bytes_written, 0);
}

TEST_F(Text_RegexTags_V7_WriteableMappedFileTest, ReadAll)
{
    ASSERT_EQ(file.OpenWrite(test_path, 512), MappedBinaryStringsFile::EWriteStatus::Success);

    std::vector<std::string_view> const expected = {"first", "", "third_record_payload"};
    for (auto const sv : expected)
    {
        ASSERT_EQ(file.Write(sv).status, MappedBinaryStringsFile::EWriteStatus::Success);
    }

    EXPECT_TRUE(file.SeekTo(0));

    std::vector<std::string_view> actual;
    while (file.GetOffset() < file.GetSize())
    {
        actual.push_back(file.ReadNext().data);
    }

    EXPECT_EQ(actual, expected);
}

TEST_F(Text_RegexTags_V7_WriteableMappedFileTest, SeekAndRead)
{
    ASSERT_EQ(file.OpenWrite(test_path, 512), MappedBinaryStringsFile::EWriteStatus::Success);

    auto const res1 = file.Write("entry_0");
    std::ignore = file.Write("entry_1");
    std::ignore = file.Write("entry_2");

    Bytes const entry_1_offset = res1.bytes_written;

    // Seek directly to second item
    EXPECT_TRUE(file.SeekTo(entry_1_offset));
    EXPECT_EQ(file.GetOffset(), entry_1_offset);

    {
        auto const read_result = file.ReadNext();
        EXPECT_EQ(read_result.status, MappedBinaryStringsFile::EReadStatus::Success);
        EXPECT_EQ(read_result.data, "entry_1");
    }

    {
        auto const read_result = file.ReadNext();
        EXPECT_EQ(read_result.status, MappedBinaryStringsFile::EReadStatus::Success);
        EXPECT_EQ(read_result.data, "entry_2");
    }
}

TEST_F(Text_RegexTags_V7_WriteableMappedFileTest, SeekOverSizeAndRead)
{
    ASSERT_EQ(file.OpenWrite(test_path, 512), MappedBinaryStringsFile::EWriteStatus::Success);
    auto const write_result = file.Write("valid_entry");

    Bytes const invalid_offset = file.GetSize() + 100;

    // Seeking past end of populated size must fail and leave offset untouched
    EXPECT_FALSE(file.SeekTo(invalid_offset));
    EXPECT_EQ(file.GetOffset(), write_result.bytes_written);

    // Reading at/past end of populated size returns empty string_view
    EXPECT_TRUE(file.SeekTo(file.GetSize()));
    auto const read_result = file.ReadNext();
    EXPECT_TRUE(read_result.data.empty());
    EXPECT_TRUE(read_result.status == MappedBinaryStringsFile::EReadStatus::EOFReached);
}
