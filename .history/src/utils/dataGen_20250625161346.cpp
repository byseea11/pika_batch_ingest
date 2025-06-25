#include "dataGen.h"
#include <fstream>
#include <random>
#include <chrono>
#include <thread>
#include <filesystem>
#include "log.h"

namespace fs = std::filesystem;
Log log;

// 构造函数，初始化配置
DataGen::DataGen(const std::string &configFilePath)
    : generator_(std::random_device()()), dist_(0, 999), keyPoolSize_(1000), maxFileSizeMB_(256)
{
    loadConfig(configFilePath);
    initializeKeyPool();
}

// 从配置文件加载配置
void DataGen::loadConfig(const std::string &configFilePath)
{
    std::ifstream configFile(configFilePath);
    configFile >> config_;
    keyPrefix_ = config_["keyPrefix"];
    valuePrefix_ = config_["valuePrefix"];
    maxFileSizeMB_ = config_["maxFileSizeMB"];
    targetSizeGB_ = config_["targetSizeGB"];
}

// 生成数据的主函数
void DataGen::generateData()
{
    size_t totalFiles = (targetSizeGB_ * 1024) / maxFileSizeMB_;
    size_t totalDataSize = targetSizeGB_ * 1024 * 1024 * 1024;

    // 使用线程池并行生成文件
    std::vector<std::thread> threads;
    for (size_t i = 0; i < totalFiles; ++i)
    {
        std::string filename = "data_" + std::to_string(i + 1) + ".json";
        threads.push_back(std::thread(&DataGen::generateFile, this, filename, totalDataSize / totalFiles));
    }

    // 等待所有线程完成
    for (auto &t : threads)
    {
        t.join();
    }

    std::cout << "Data generation completed!" << std::endl;
}

// 生成指定大小的文件
void DataGen::generateFile(const std::string &filename, size_t fileSize)
{
    std::ofstream outputFile(filename);
    size_t currentFileSize = 0;

    while (currentFileSize < fileSize)
    {
        std::string key = generateKey();
        std::string value = valuePrefix_ + std::to_string(dist_(generator_));

        size_t kvPairSize = key.size() + value.size();
        outputFile << key << ":" << value << "\n";

        currentFileSize += kvPairSize;
    }

    outputFile.close();
    std::cout << "File " << filename << " generated with size: " << currentFileSize / (1024 * 1024) << " MB." << std::endl;
}

void DataGen::ensureKeyPoolNotEmpty()
{
    std::unique_lock<std::mutex> lock(poolMutex_);
    if (keyPool_.empty())
        expandKeyPool();
}

std::string DataGen::generateKey()
{
    ensureKeyPoolNotEmpty(); // 确保键池非空（内部已加锁）
                             // 随机选择一个键
    LOG_INFO("select dist number is: " + std::to_string(dist_(generator_)) + " , keyPool of size: " + std::to_string(keyPool_.size()));
    size_t index = dist_(generator_) % keyPool_.size(); // 此处仍需锁
    return keyPool_[index];
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
    keyPool_.clear();
    for (int i = 0; i < keyPoolSize_; ++i)
    {
        keyPool_.push_back(keyPrefix_ + std::to_string(i));
    }

    poolCond_.notify_all(); // 初始化完成后通知所有等待的线程
}