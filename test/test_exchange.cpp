#include <gtest/gtest.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include "exchange/JsonFileManager.h"
#include "utils/kvEntry.h" // 如果 KvEntry 定义在这个头文件中

using json = nlohmann::json;

class ExchangeTest : public ::testing::Test
{
protected:
    std::string jsonFilePath = "test_data.json";

    void SetUp() override
    {
        json data = json::array({{{"key", "key_1"}, {"value", "value_abc"}, {"expire", 1719820385}},
                                 {{"key", "key_2"}, {"value", "value_def"}}});

        std::ofstream out(jsonFilePath);
        ASSERT_TRUE(out.is_open()) << "Failed to create test_data.json";
        out << data.dump(4);
        out.close();
    }

    void TearDown() override
    {
        std::remove(jsonFilePath.c_str());
    }
};

TEST_F(ExchangeTest, JsonFileManagerParseShouldSupportExpireField)
{
    JsonFileManager manager;
    DataType result = manager.parse(jsonFilePath);

    ASSERT_EQ(result.size(), 2);

    EXPECT_EQ(result[0].key, "key_1");
    EXPECT_EQ(result[0].value, "value_abc");
    EXPECT_EQ(result[0].timestamp, 1719820385);

    EXPECT_EQ(result[1].key, "key_2");
    EXPECT_EQ(result[1].value, "value_def");
    EXPECT_EQ(result[1].timestamp, 0); // 默认值
}
