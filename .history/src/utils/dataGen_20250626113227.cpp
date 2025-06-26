#include "dataGen.h"
#include <fstream>
#include <random>
#include <chrono>
#include <thread>
#include <filesystem>
#include "log.h"
#include <ThreadPool.h>
#include <shared_mutex>

namespace fs = std::filesystem;
Log log;

// 构造函数，初始化配置
DataGen::DataGen(const std::string &configFilePath, const std::string &filePath)
    : fileManager_(filePath), keyPoolSize_(4000), maxFileSizeMB_(256)
{
    if (loadConfig(configFilePath).isError())
    {
        LOG_ERROR("Failed to load configuration from " + configFilePath);
        throw std::runtime_error("Failed to load configuration from " + configFilePath);
    }
    initializeKeyPool();
}

// 从配置文件加载配置
Result DataGen::loadConfig(const std::string &configFilePath)
{
    std::ifstream configFile(configFilePath);
    configFile >> config_;
    targetSizeGB_ = config_["targetSizeGB"];
    maxSizeGB_ = config_["maxSizeGB"];
    if (targetSizeGB_ > maxSizeGB_)
    {
        LOG_ERROR("Target size cannot be greater than maximum size.");
        return Result(Result::Ret::kConfigError, "Target size cannot be greater than maximum size.");
    }
    keyPrefix_ = config_["keyPrefix"];
    valuePrefix_ = config_["valuePrefix"];
    maxFileSizeMB_ = config_["maxFileSizeMB"];
    return Result(Result::Ret::kOk, "Configuration loaded successfully.");
}

// 生成数据的主函数
/**
 * 1. 分割需要多少个文件，然后多线程处理每个文件
 * 2. 每个文件中的逻辑就是使用vector插入数据，最后sort，然后调用file.write写入文件
 */
void DataGen::generateData()
{
    size_t totalFiles = (targetSizeGB_ * 1024) / maxFileSizeMB_;
    size_t totalDataSize = targetSizeGB_ * 1024 * 1024 * 1024;
    size_t perFileDataSize = totalDataSize / totalFiles;
    size_t remainder = totalDataSize % totalFiles;

    // 初始化键池
    initializeKeyPool();

    startKeyPoolUpdateTask();

    // 创建线程池
    ThreadPool pool(numThreads_);

    // 提交文件生成任务到线程池
    for (size_t i = 1; i < totalFiles; ++i)
    {
        pool.enqueue([this, perFileDataSize]
                     { this->generateFile(perFileDataSize); });
    }

    // 最后一个线程处理 remainder
    pool.enqueue([this, perFileDataSize, remainder]
                 { this->generateFile(perFileDataSize + remainder); });

    // 不需要 join，析构 pool 会自动等所有任务完成
    LOG_INFO("Data generation completed. Total files generated: " + std::to_string(totalFiles) +
             ", Total data size: " + std::to_string(totalDataSize) + " bytes.");
}

// 生成指定大小的文件
void DataGen::generateFile(size_t fileSize)
{
    std::vector<std::unordered_map<std::string, std::string>> data;

    size_t approxEntrySize = 50; // 估算每条键值对大小（字节），可根据实际情况微调
    size_t numEntries = fileSize / approxEntrySize;

    // 为每个线程创建独立的随机生成器和分布器
    std::random_device rd;
    std::mt19937 gen(rd());                                             // 线程局部随机数生成器
    std::uniform_int_distribution<size_t> dist(0, keyPool_.size() - 1); // 线程局部分布器

    for (size_t i = 0; i < numEntries; ++i)
    {
        std::unordered_map<std::string, std::string> entry;
        entry["key"] = keyPrefix_ + generateKey();
        entry["value"] = valuePrefix_ + std::to_string(dist(gen));
        data.push_back(entry);
    }

    std::sort(data.begin(), data.end(), [](const auto &a, const auto &b)
              { return a.at("key") < b.at("key"); });

    fileManager_.write(data);
}

// 确保键池非空
void DataGen::ensureKeyPoolNotEmpty()
{
    std::unique_lock<std::mutex> lock(poolMutex_);
    if (keyPool_.empty())
        expandKeyPool();
}
// 随机从键池中选择一个键
std::string DataGen::generateKey()
{
    std::shared_lock<std::shared_mutex> lock(poolMutex_);
    if (keyPool_.empty())
    {
        LOG_ERROR("Key pool is unexpectedly empty");
        return "invalid_key";
    }

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, keyPool_.size() - 1);
    return keyPool_[dist(gen)];
}

// 动态扩展键池
void DataGen::expandKeyPool()
{
    std::unique_lock<std::mutex> lock(poolMutex_);

    // 等待直到键池大小不小于 keyPoolSize_
    while (keyPool_.size() < keyPoolSize_)
    {
        poolCond_.wait(lock); // 等待条件变量，释放锁并阻塞当前线程
    }

    size_t initialSize = keyPool_.size();
    for (size_t i = initialSize; i < initialSize + keyPoolSize_; ++i)
    {
        keyPool_.push_back(keyPrefix_ + std::to_string(i));
    }

    poolCond_.notify_all(); // 通知其他线程键池已经扩展
}

void DataGen::initializeKeyPool()
{
    std::unique_lock<std::shared_mutex> lock(poolMutex_);
    keyPool_.clear();
    for (int i = 0; i < keyPoolSize_; ++i)
    {
        keyPool_.push_back(keyPrefix_ + std::to_string(i));
    }
    LOG_INFO("Key pool initialized with size: " + std::to_string(keyPool_.size()));
}

void DataGen::rebuildKeyPool()
{
    std::unique_lock<std::shared_mutex> lock(poolMutex_);
    keyPool_.clear();
    for (size_t i = 0; i < numThreads_ * keyPoolSize_; ++i)
    {
        keyPool_.push_back(keyPrefix_ + std::to_string(i));
    }
    LOG_INFO("Key pool rebuilt with size: " + std::to_string(keyPool_.size()));
    poolCond_.notify_all();
}

void DataGen::startKeyPoolUpdateTask()
{
    updateThread_ = std::thread([this]()
                                {
        while (!stopUpdateThread_) {
            std::this_thread::sleep_for(poolUpdateInterval_);
            rebuildKeyPool();
        } });
}
