#ifndef DATAGEN_H
#define DATAGEN_H
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

using json = nlohmann::json;

class DataGen
{
public:
    // 构造函数，初始化
    DataGen(const std::string &configFilePath);

    // 生成数据的主函数
    void generateData();

private:
    // 生成文件的函数
    void generateFile(const std::string &filename, size_t fileSize);

    // 随机从键池中选择一个键
    std::string generateKey();

    // 动态扩展键池
    void expandKeyPool();

    // 初始化键池
    void initializeKeyPool();
    void loadConfig(const std::string &configFilePath);

private:
    std::default_random_engine generator_;    // 随机数生成器
    std::uniform_int_distribution<int> dist_; // 随机数分布
    std::vector<std::string> keyPool_;        // 键池
    json config_;                             // 配置文件内容
    std::string keyPrefix_;                   // 键前缀
    std::string valuePrefix_;                 // 值前缀
    size_t keyPoolSize_;                      // 键池大小
    size_t maxFileSizeMB_;                    // 每个文件的最大大小（MB）
    size_t targetSizeGB_;                     // 目标数据大小（GB）

    std::mutex poolMutex_;             // 键池的互斥锁
    std::condition_variable poolCond_; // 键池扩展的条件变量
};

#define LOG_WARN(content) info_print(AOF_LOG_WARN, content)
#define LOG_INFO(content) info_print(AOF_LOG_INFO, content)
#define LOG_TRACE(content) info_print(AOF_LOG_TRACE, content)
#define LOG_DEBUG(content) info_print(AOF_LOG_DEBUG, content)

void info_print(const std::string &content)
{
    if (l > AOF_LOG_FATAL || l < AOF_LOG_DEBUG || content.empty())
    {
        return;
    }

    if (l < aof_info_level_)
    {
        return;
    }
    if (l >= AOF_LOG_ERR)
    {
        std::cerr << content << std::endl;
    }
    else
    {
        std::cout << content << std::endl;
    }
}
#endif