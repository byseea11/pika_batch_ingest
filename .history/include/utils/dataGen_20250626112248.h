#ifndef DATAGEN_H
#define DATAGEN_H
#include "fileManager.h"
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <random>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <json.hpp>
#include <atomic>

using json = nlohmann::json;

class DataGen
{
public:
    // 构造函数，初始化
    DataGen(const std::string &configFilePath, const std::string &filePath);

    ~DataGen()
    {
        stopUpdateThread_ = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 给后台线程时间退出
    }

    // 生成数据的主函数
    void generateData();

private:
    // 生成文件的函数
    void generateFile(size_t fileSize);

    // 随机从键池中选择一个键
    std::string generateKey();

    // 动态扩展键池
    void ensureKeyPoolNotEmpty();
    void expandKeyPool();

    // 初始化键池
    void initializeKeyPool();
    Result loadConfig(const std::string &configFilePath);

    void rebuildKeyPool();
    void startKeyPoolUpdateTask();

private:
    FileManager fileManager_;                                                   // 文件管理器实例
    std::vector<std::string> keyPool_;                                          // 键池
    json config_;                                                               // 配置文件内容
    std::string keyPrefix_;                                                     // 键前缀
    std::string valuePrefix_;                                                   // 值前缀
    size_t keyPoolSize_;                                                        // 键池大小
    size_t maxFileSizeMB_;                                                      // 每个文件的最大大小（MB）
    size_t targetSizeGB_;                                                       // 目标数据大小（GB）
    size_t maxSizeGB_;                                                          // 最大数据大小（GB）
    std::chrono::minutes poolUpdateInterval_;                                   // 键池更新的时间间隔
    size_t numThreads_ = std::max(1u, std::thread::hardware_concurrency() - 1); // 线程数，至少1个线程
    std::atomic<bool> stopUpdateThread_{false};
    std::thread updateThread_; // 后台线程用于定期更新键池

    std::mutex poolMutex_;             // 键池的互斥锁
    std::condition_variable poolCond_; // 键池扩展的条件变量
};

#endif