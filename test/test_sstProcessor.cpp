#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "exchange/sstProcessor.h"
#include "utils/result.h"
#include "exchange/JsonFileManager.h"
#include <sstream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using ::testing::_;
using ::testing::Return;
using ::testing::Throw;

// 使用 Google Mock 来模拟 JsonFileManagerBase 类
class MockJsonFileManager : public JsonFileManagerBase
{
public:
    MOCK_METHOD(DataType, parse, (const std::string &), (override)); // Mock 解析方法
};

// 测试夹具，用于为每个测试创建共享的环境
class SstProcessorTest : public ::testing::Test
{
protected:
    // 初始化设置
    void SetUp() override
    {
        options_.create_if_missing = true;
        cfh_ = nullptr; // 使用默认 ColumnFamily
        sstProcessor_ = std::make_unique<SstProcessor>(options_, cfh_);
    }

    // 清理
    void TearDown() override
    {
        // 清理临时文件
        if (std::filesystem::exists(tempJsonFile_))
        {
            std::filesystem::remove(tempJsonFile_);
        }
    }

    // 成员变量
    rocksdb::Options options_;
    rocksdb::ColumnFamilyHandle *cfh_;
    std::unique_ptr<SstProcessor> sstProcessor_;
    std::string tempJsonFile_; // 临时 JSON 文件路径
};

// JSON 数据（模拟文件内容）
const std::string kTestJson = R"(
[
    {"key": "key_30095", "value": "value_31642"},
    {"key": "key_30407", "value": "value_16672"},
    {"expire": 1751100885, "key": "key_31010", "value": "value_28266"},
    {"expire": 1751102496, "key": "key_31132", "value": "value_1848"}
]
)";

// 模拟 JSON 解析
DataType MockParseJson(const std::string &jsonStr)
{
    DataType data;
    std::istringstream ss(jsonStr);
    nlohmann::json root;
    ss >> root; // 使用正确的 JSON 解析方法

    for (const auto &entry : root)
    {
        DataType::value_type kv;
        kv.key = entry["key"].get<std::string>();
        kv.value = entry["value"].get<std::string>();
        data.push_back(kv);
    }
    return data;
}

// 测试: 检查 processSstFile 是否正确处理 JSON 文件
TEST_F(SstProcessorTest, TestProcessSstFile)
{
    // 写入临时 JSON 文件
    tempJsonFile_ = "test.json";
    std::ofstream outFile(tempJsonFile_);
    outFile << kTestJson;
    outFile.close();

    // Mock JsonFileManager 解析过程
    MockJsonFileManager mockFileManager;
    EXPECT_CALL(mockFileManager, parse(testing::_))
        .WillOnce(Return(MockParseJson(kTestJson))); // 使用 Mock 解析

    std::string inputJsonPath = tempJsonFile_;     // 使用临时文件路径
    std::string outputSstPath = "test_output.sst"; // 输出路径

    // 调用 processSstFile
    Result result = sstProcessor_->processSstFile(&mockFileManager, inputJsonPath, outputSstPath);

    // 验证成功的返回结果
    EXPECT_EQ(result.getRet(), Result::Ret::kOk);
    EXPECT_EQ(result.message_raw(), "SST file created successfully: test_output.sst");

    // 删除临时文件
    std::filesystem::remove(tempJsonFile_);
    std::filesystem::remove(outputSstPath);
}

// 测试: 模拟 JSON 解析失败时的错误处理
TEST_F(SstProcessorTest, TestProcessSstFileErrorParsingJson)
{
    // 写入临时 JSON 文件
    tempJsonFile_ = "test.json";
    std::ofstream outFile(tempJsonFile_);
    outFile << kTestJson;
    outFile.close();

    MockJsonFileManager mockFileManager;
    // Mock 解析失败
    EXPECT_CALL(mockFileManager, parse(testing::_))
        .WillOnce(Throw(std::runtime_error("JSON parse error"))); // 模拟解析错误

    std::string inputJsonPath = tempJsonFile_;     // 使用临时文件路径
    std::string outputSstPath = "test_output.sst"; // 输出路径

    // 调用 processSstFile
    Result result = sstProcessor_->processSstFile(&mockFileManager, inputJsonPath, outputSstPath);

    // 验证错误结果
    EXPECT_EQ(result.getRet(), Result::Ret::kFileReadError);
    EXPECT_EQ(result.message_raw(), "JSON parse failed: JSON parse error");

    // 删除临时文件
    std::filesystem::remove(tempJsonFile_);
    std::filesystem::remove(outputSstPath);
}
