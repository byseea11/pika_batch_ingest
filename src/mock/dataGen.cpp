#include "mock/dataGen.h"
#include <fstream>
#include <random>
#include <chrono>
#include <thread>
#include <filesystem>
#include "utils/klog.h"
#include <ThreadPool.h>
#include <shared_mutex>
#include <vector>
#include <unordered_map>
#include "mock/fileManager.h"
#include "utils/compare.h"

namespace fs = std::filesystem;

std::string thread_id_to_string(std::thread::id id)
{
    std::ostringstream oss;
    oss << id; // 使用重载的 << 运算符
    return oss.str();
}

// 构造函数，初始化配置
DataGen::DataGen(const std::string &configFilePath, const std::string &dicPath)
    : fileManager_(std::make_shared<FileManager>(dicPath)), keyPoolSize_(40000), maxFileSizeMB_(256)
{
    if (loadConfig(configFilePath).isError())
    {
        LOG_ERROR("Failed to load configuration from " + configFilePath);
        throw std::runtime_error("Failed to load configuration from " + configFilePath);
    }
    // 初始化键池
    if (initializeKeyPool().isError())
    {
        LOG_ERROR("Failed to initialize key pool.");
        throw std::runtime_error("Failed to initialize key pool.");
    }
}

// 从配置文件加载配置
Result DataGen::loadConfig(const std::string &configFilePath)
{
    std::ifstream configFile(configFilePath);
    configFile >> config_;
    targetSizeMB_ = config_["targetSizeMB"];
    maxSizeMB_ = config_["maxSizeGB"].get<double>() * 1024;
    if (targetSizeMB_ > maxSizeMB_)
    {
        LOG_ERROR("Target size cannot be greater than maximum size.");
        return Result(Result::Ret::kConfigError, "Target size cannot be greater than maximum size.");
    }
    keyPrefix_ = config_["keyPrefix"];
    valuePrefix_ = config_["valuePrefix"];
    maxFileSizeMB_ = config_["maxFileSizeMB"];
    approxEntrySizeKB_ = config_["approxEntrySizeKB"];
    if (maxFileSizeMB_ <= 0 || approxEntrySizeKB_ <= 0)
    {
        LOG_ERROR("Max file size and approx entry size must be greater than zero.");
        return Result(Result::Ret::kConfigError, "Max file size and approx entry size must be greater than zero.");
    }

    return Result(Result::Ret::kOk, "Configuration loaded successfully.");
}

// 生成数据的主函数
/**
 * 1. 分割需要多少个文件，然后多线程处理每个文件
 * 2. 每个文件中的逻辑就是使用vector插入数据，最后sort，然后调用file.write写入文件
 */
Result DataGen::generateData()
{
    // 使用浮点数，添加范围检查
    double totalDataSize = targetSizeMB_;
    double totalFiles = (maxFileSizeMB_ > 0) ? totalDataSize / maxFileSizeMB_ : 0.0;
    double remainder = (maxFileSizeMB_ > 0) ? std::fmod(totalDataSize, maxFileSizeMB_) : 0.0;
    double perFileDataSize = maxFileSizeMB_; // 每个文件的大小（MB）

    LOG_INFO("Total data size: " + std::to_string(totalDataSize) +
             " MB, Total files to generate: " + std::to_string(totalFiles) +
             ", Remainder: " + std::to_string(remainder) + " MB.");

    // 创建线程池
    ThreadPool pool(numThreads_);

    LOG_INFO("numThreads: " + std::to_string(numThreads_));

    // 提交文件生成任务到线程池
    for (size_t i = 1; i < totalFiles; ++i)
    {
        pool.enqueue([this, perFileDataSize]
                     { this->generateFile(perFileDataSize); });
    }

    // 最后一个线程处理 remainder
    pool.enqueue([this, perFileDataSize, remainder]
                 { this->generateFile(perFileDataSize + remainder); });

    return Result(Result::Ret::kOk, "Data generation completed successfully.");
}

// 生成指定大小的文件
Result DataGen::generateFile(size_t fileSize)
{
    DataType data;
    Result res;

    size_t numEntries = fileSize * 1024 / approxEntrySizeKB_; // 计算每个文件需要多少条数据

    // 为每个线程创建独立的随机生成器和分布器
    std::random_device rd;
    std::mt19937 gen(rd());                                             // 线程局部随机数生成器
    std::uniform_int_distribution<size_t> dist(0, keyPool_.size() - 1); // 线程局部分布器

    for (size_t i = 0; i < numEntries; ++i)
    {
        KvEntry entry;
        try
        {
            entry.key = generateKey().message_raw(); // 你要确保返回的是 string
            entry.value = valuePrefix_ + std::to_string(dist(gen));
            entry.timestamp = generateRandomTimestamp();
            data.emplace_back(entry);
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Failed to generate key: " + std::string(e.what()));
            rebuildKeyPool(); // 如果发生异常，重新初始化键池
            --i;              // 重试当前条目
        }
    }

    std::sort(data.begin(), data.end(), ComparePair());

    if (data.empty())
    {
        LOG_WARN("No data generated for file, skipping write.");
        return Result(Result::Ret::kOk, "No data generated for file.");
    }
    res = fileManager_->write(data);
    if (res.isError())
    {
        LOG_ERROR("File write error : " + res.message());
        return Result(Result::Ret::kFileWriteError, res.message());
    }
    return Result(Result::Ret::kOk, "File generated successfully.");
}

// 随机从键池中选择一个键
Result DataGen::generateKey()
{
    std::shared_lock<std::shared_mutex> lock(poolMutex_);
    if (keyPool_.empty())
    {
        LOG_ERROR("Key pool is unexpectedly empty");
        throw std::runtime_error("Key pool is unexpectedly empty");
    }

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, keyPool_.size() - 1);
    LOG_INFO("Thread ID: " + thread_id_to_string(std::this_thread::get_id()) + " Generating key from pool of size: " + std::to_string(keyPool_.size()));
    return Result(Result::Ret::kOk, keyPool_[dist(gen)]);
}

Result DataGen::initializeKeyPool()
{
    std::unique_lock<std::shared_mutex> lock(poolMutex_);
    keyPool_.clear();
    for (int i = 0; i < keyPoolSize_; ++i)
    {
        keyPool_.push_back(keyPrefix_ + std::to_string(i));
    }
    LOG_INFO("Key pool initialized with size: " + std::to_string(keyPool_.size()));
    return Result(Result::Ret::kOk, "Key pool initialized successfully.");
}
Result DataGen::rebuildKeyPool()
{
    std::vector<std::string> newPool; // 临时容器
    for (size_t i = 0; i < numThreads_ * keyPoolSize_; ++i)
    {
        newPool.push_back(keyPrefix_ + std::to_string(i));
    }
    {
        std::unique_lock<std::shared_mutex> lock(poolMutex_);
        keyPool_.swap(newPool); // 原子替换
    }
    LOG_INFO("Key pool rebuilt with size: " + std::to_string(keyPool_.size()));
    return Result::kOk;
}

// Result DataGen::startKeyPoolUpdateTask()
// {
//     updateThread_ = std::thread([this]()
//                                 {
//         while (!stopUpdateThread_) {
//             std::this_thread::sleep_for(poolUpdateInterval_);
//             rebuildKeyPool();
//         } });
//     return Result(Result::Ret::kOk, "Key pool update task started successfully.");
// }

// Result DataGen::stopUpdateThread()
// {
//     stopUpdateThread_ = true;
//     if (updateThread_.joinable())
//     {
//         updateThread_.join(); // 等待线程安全退出
//     }
//     return Result(Result::Ret::kOk, "Key pool update task stopped successfully.");
// }

std::vector<std::string> &
DataGen::getKeyPool()
{
    std::shared_lock<std::shared_mutex> lock(poolMutex_);
    return keyPool_;
}