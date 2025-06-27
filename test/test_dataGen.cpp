#include <gtest/gtest.h>
#include "mock/dataGen.h"
#include <fstream>
#include <filesystem>
#include "gmock/gmock.h"
#include "mock/fileManager.h"
#include "utils/kvEntry.h"

// 创建一个 MockFileManager 类，继承自 FileManagerBase
class MockFileManager : public FileManagerBase
{
public:
    MOCK_METHOD(Result, write, (const DataType &data), (override));
};

class DataGenTest : public ::testing::Test
{
protected:
    std::unique_ptr<DataGen> gen;
    std::shared_ptr<MockFileManager> mockFileManager = std::make_shared<MockFileManager>();
    const std::string configPath = "test_config.json";
    const std::string outputDir = "test_output";

    void SetUp() override
    {
        // 创建测试配置文件
        std::ofstream config(configPath);
        config << R"({
            "targetSizeMB": 100,
            "maxSizeGB": 2,
            "keyPrefix": "key_",
            "valuePrefix": "val_",
            "maxFileSizeMB": 20,
            "approxEntrySizeKB": 50
        })";
        config.close();
        gen = std::make_unique<DataGen>(configPath, outputDir);
        // 确保输出目录存在
        std::filesystem::create_directories(outputDir);
    }

    void TearDown() override
    {
        std::filesystem::remove(configPath);
        std::filesystem::remove_all(outputDir);
    }
};
/**
 * 测试生成键的功能
 * 测试键在池中是否存在
 * 测试键池为空时的行为
 */

TEST_F(DataGenTest, GenerateKeyShouldReturnNonEmptyKey)
{
    Result key = gen->generateKey();
    EXPECT_FALSE(key.message_raw().empty());
}

TEST_F(DataGenTest, GenerateKeyShouldBeInKeyPool)
{
    Result key = gen->generateKey();
    const auto &pool = gen->getKeyPool();
    EXPECT_TRUE(std::find(pool.begin(), pool.end(), key.message_raw()) != pool.end());
}

TEST_F(DataGenTest, GenerateKeyShouldHandleEmptyPool)
{
    gen->getKeyPool().clear();
    EXPECT_THROW({ gen->generateKey(); }, std::runtime_error);
}

/**
 * 正常情况下生成文件
 * fileSize == 0 时行为
 * approxEntrySizeKB_ 设置不合理（如0）
 * 键池为空
 * 写入操作失败时（mock fileManager_）
 * 写入后的文件是否排序
 */

TEST_F(DataGenTest, GenerateFileShouldGenerateCorrectNumberOfEntriesAndSorted)
{
    gen = std::make_unique<DataGen>(configPath, outputDir);
    gen->setFileManager(mockFileManager);
    EXPECT_CALL(*mockFileManager, write(testing::_))
        .WillOnce([](const DataType &data)
                  {
            EXPECT_FALSE(data.empty());
            EXPECT_EQ(data.size(), 100);
            for (size_t i = 1; i < data.size(); ++i)
            {
                EXPECT_LE(data[i - 1].key, data[i].key); // 按 key 排序
            }

            return Result(Result::Ret::kOk, "File generated successfully."); });

    gen->generateFile(100);
}

TEST_F(DataGenTest, GenerateFileShouldHandleZeroFileSize)
{
    gen = std::make_unique<DataGen>(configPath, outputDir);
    gen->setFileManager(mockFileManager);
    EXPECT_CALL(*mockFileManager, write(testing::_)).Times(0);
    gen->generateFile(0);
}

TEST_F(DataGenTest, GenerateFileShouldNotCrashIfEntrySizeIsZero)
{
    std::string configPath = "test_zeroapp_config.json";
    std::ofstream config(configPath);
    config << R"({
        "targetSizeMB": 100,
        "maxSizeGB": 2,
        "keyPrefix": "key_",
        "valuePrefix": "val_",
        "maxFileSizeMB": 20,
        "approxEntrySizeKB": 0
    })";
    config.close();
    EXPECT_THROW({
        gen = std::make_unique<DataGen>(configPath, outputDir);
        gen->setFileManager(mockFileManager);
        gen->generateFile(100); }, std::runtime_error);
    EXPECT_CALL(*mockFileManager, write(testing::_)).Times(0);
    std::filesystem::remove("test_zeroapp_config.json");
}

TEST_F(DataGenTest, GenerateFileShouldHandleEmptyKeyPool)
{
    gen = std::make_unique<DataGen>(configPath, outputDir);
    gen->setFileManager(mockFileManager);

    gen->getKeyPool().clear();
    EXPECT_CALL(*mockFileManager, write(testing::_))
        .WillOnce(testing::Return(Result(Result::Ret::kOk, "File generated successfully.")));
    EXPECT_EQ(gen->generateFile(100).getRet(), Result::Ret::kOk);
}

TEST_F(DataGenTest, GenerateFileShouldHandleWriteFailure)
{
    gen = std::make_unique<DataGen>(configPath, "output");
    gen->setFileManager(mockFileManager);
    // 确保生成至少一个 entry，否则不会触发 write 调用
    EXPECT_CALL(*mockFileManager, write(testing::_))
        .WillOnce(testing::Return(Result(Result::Ret::kFileOpenError, "output/data_1.json")));

    Result res = gen->generateFile(100); // 改为更大的值以确保写入
    EXPECT_EQ(res.getRet(), Result::Ret::kFileWriteError);
}

/**
 * 测试分割文件
 */
TEST_F(DataGenTest, GenerateDataSplitFiles)
{
    std::string configPath = "test_split_config.json";
    std::ofstream config(configPath);
    config << R"({
        "targetSizeMB": 100,
        "maxSizeGB": 2,
        "keyPrefix": "key_",
        "valuePrefix": "val_",
        "maxFileSizeMB": 20,
        "approxEntrySizeKB": 50
    })";
    config.close();
    if (!std::filesystem::exists(outputDir))
    {
        std::filesystem::create_directories(outputDir);
    }
    gen = std::make_unique<DataGen>(configPath, outputDir);
    gen->setFileManager(mockFileManager);
    EXPECT_CALL(*mockFileManager, write(testing::_)).WillRepeatedly([](const DataType &data)
                                                                    {
                      EXPECT_FALSE(data.empty());
                      return Result(Result::Ret::kOk, "File generated successfully."); });
    EXPECT_EQ(gen->generateData().getRet(), Result::Ret::kOk);
    std::filesystem::remove("test_split_config.json");
}

/**
 * 测试多线程重建键池
 * 键池内容验证​
 * 测试竟态条件
 * 定时任务触发模拟
 */
TEST_F(DataGenTest, RebuildKeyPoolConcurrentRebuildKeyPool)
{
    DataGen gen(configPath, outputDir);
    const int threadCount = gen.getNumThreads();
    size_t initialSize = gen.getKeyPool().size();
    std::vector<std::thread> threads;
    for (int i = 0; i < threadCount; ++i)
    {
        threads.emplace_back([&gen]
                             { gen.rebuildKeyPool(); });
    }
    for (auto &t : threads)
        t.join();
    EXPECT_EQ(gen.getKeyPool().size(), initialSize * threadCount);
}

TEST_F(DataGenTest, RebuildKeyKeyPoolContent)
{
    DataGen gen(configPath, outputDir);
    gen.rebuildKeyPool();
    auto pool = gen.getKeyPool();
    for (size_t i = 0; i < pool.size(); ++i)
    {
        EXPECT_FALSE(pool[i].size() == 0);
    }
}

TEST_F(DataGenTest, RebuildKeyReadDuringRebuild)
{
    DataGen gen(configPath, outputDir);
    std::atomic<bool> stop(false);
    std::thread updater([&gen, &stop]
                        {
        while (!stop) gen.rebuildKeyPool(); });
    std::vector<std::thread> readers;
    for (int i = 0; i < 5; ++i)
    {
        readers.emplace_back([&gen, &stop]
                             {
            while (!stop) {
                auto pool = gen.getKeyPool(); 
                EXPECT_FALSE(pool.empty());
            } });
    }
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 测试运行1秒
    stop = true;
    updater.join();
    for (auto &t : readers)
        t.join();
}

// TEST_F(DataGenTest, RebuildKeyKeyPoolUpdateInterval)
// {
//     DataGen gen(configPath, outputDir);
//     auto initialSize = gen.getKeyPool().size(); // 直接获取当前键池大小
//     gen.startKeyPoolUpdateTask();
//     std::this_thread::sleep_for(std::chrono::seconds(3));
//     auto updatedSize = gen.getKeyPool().size(); // 重新获取最新键池大小
//     EXPECT_NE(initialSize, updatedSize);        // 比较最新状态
//     gen.stopUpdateThread();
// }