#include <gtest/gtest.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include "exchange/JsonFileManager.h"

using json = nlohmann::json;

class ExchangeTest : public ::testing::Test
{
protected:
    std::string jsonFilePath = "test_data.json";

    void SetUp() override
    {
        json data = json::array({{{"key", "key_999"}, {"value", "value_942"}},
                                 {{"key", "key_999"}, {"value", "value_975"}},
                                 {{"key", "key_999"}, {"value", "value_895"}}});

        std::ofstream out(jsonFilePath);
        ASSERT_TRUE(out.is_open()) << "Failed to create test_data.json";
        out << data.dump(4); // 格式化写入，4空格缩进
        out.close();
    }

    void TearDown() override
    {
        std::remove(jsonFilePath.c_str()); // 清理测试文件
    }
};

TEST_F(ExchangeTest, LoadJsonArrayFromFile)
{
    JsonFileManager manager;
    json result = manager.load("test_data.json");

    ASSERT_TRUE(result.is_array());
    ASSERT_EQ(result.size(), 3);

    EXPECT_EQ(result[0]["key"], "key_999");
    EXPECT_EQ(result[1]["value"], "value_975");
    EXPECT_EQ(result[2]["value"], "value_895");
}
