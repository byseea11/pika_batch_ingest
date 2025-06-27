#ifndef DATAGEN_H
#define DATAGEN_H
#include "mock/fileManager.h"
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <random>
#include <string>
#include <vector>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <shared_mutex>
#include <mutex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class DataGen
{
public:
    // 构造函数，初始化
    DataGen(const std::string &configFilePath, const std::string &dicPath);

    ~DataGen()
    {
        // stopUpdateThread();
    }

    // 生成数据的主函数
    Result generateData();

    // private:
    // 生成文件的函数
    Result generateFile(size_t fileSize);

    // 随机从键池中选择一个键
    Result generateKey();

    // 动态扩展键池
    Result ensureKeyPoolNotEmpty();
    Result expandKeyPool();

    // 初始化键池
    Result initializeKeyPool();
    Result loadConfig(const std::string &configFilePath);

    Result rebuildKeyPool();
    // Result startKeyPoolUpdateTask();
    // Result stopUpdateThread();

    // 获取当前键池用于测试
    std::vector<std::string> &getKeyPool();
    size_t getNumThreads() const { return numThreads_; }
    void setFileManager(const std::shared_ptr<FileManagerBase> &fileManager) { fileManager_ = std::move(fileManager); }

private:
    std::shared_ptr<FileManagerBase> fileManager_;
    // 文件管理器实例
    std::vector<std::string> keyPool_; // 键池
    json config_;                      // 配置文件内容
    std::string keyPrefix_;            // 键前缀
    std::string valuePrefix_;          // 值前缀
    size_t keyPoolSize_;               // 键池大小
    double maxFileSizeMB_;             // 每个文件的最大大小（MB）
    double targetSizeMB_;              // 目标数据大小（MB）
    double maxSizeMB_;                 // 最大数据大小（MB）
    double approxEntrySizeKB_;         // 每个条目的平均大小（KB）
    // std::chrono::seconds poolUpdateInterval_ = std::chrono::seconds(1);         // 键池更新的时间间隔
    size_t numThreads_ = std::max(1u, std::thread::hardware_concurrency() - 1); // 线程数，至少1个线程
    std::atomic<bool> stopUpdateThread_{false};
    std::thread updateThread_; // 后台线程用于定期更新键池

    std::shared_mutex poolMutex_; // 键池的互斥锁
    friend class DataGenTest;     // 允许测试类访问私有成员
};

#endif