#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include "utils/compare.h" // 包含 CompareString 和 ComparePair
#include "utils/kvEntry.h" // 包含 KvEntry 定义

// --------------------- ComparePair Tests with Fixture ---------------------
class ComparePairTest : public ::testing::Test
{
protected:
    ComparePair cmp;
    std::vector<KvEntry> data;

    void SetUp() override
    {
        data = {
            {"key_10", "value_10", 300},
            {"key_2", "value_20", 100},
            {"key_1", "value_abc", 500},
            {"key_100", "value_5", 50},
            {"key_10", "value_10", 100}, // same key/value, different timestamp
            {"key_10", "value_5", 900}};
    }
};

TEST_F(ComparePairTest, PrimaryKeyLengthThenLex)
{
    EXPECT_TRUE(cmp({"key_1", "x", 0}, {"key_10", "x", 0}));   // key_1 < key_10 (length)
    EXPECT_FALSE(cmp({"key_100", "x", 0}, {"key_2", "x", 0})); // key_100 > key_2
}

TEST_F(ComparePairTest, TieBreakOnValue)
{
    EXPECT_TRUE(cmp({"key_10", "a", 0}, {"key_10", "b", 0}));
    EXPECT_FALSE(cmp({"key_10", "z", 0}, {"key_10", "a", 0}));
    EXPECT_FALSE(cmp({"key_10", "same", 0}, {"key_10", "same", 0}));
}

TEST_F(ComparePairTest, SortKvEntryVector)
{
    std::sort(data.begin(), data.end(), cmp);

    std::vector<KvEntry> expected = {
        {"key_1", "value_abc", 500}, // key len: 6
        {"key_2", "value_20", 100},  // key len: 6
        {"key_10", "value_5", 900},  // key len: 7
        {"key_10", "value_10", 100},
        {"key_10", "value_10", 300},
        {"key_100", "value_5", 50}, // key len: 8
    };

    ASSERT_EQ(data.size(), expected.size());
    for (size_t i = 0; i < data.size(); ++i)
    {
        EXPECT_EQ(data[i].key, expected[i].key);
        EXPECT_EQ(data[i].value, expected[i].value);
        EXPECT_EQ(data[i].timestamp, expected[i].timestamp);
    }
}
