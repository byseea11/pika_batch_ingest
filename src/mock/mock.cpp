#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <stdexcept>
#include "utils/klog.h"
#include "mock/dataGen.h"
#include "utils/result.h"
#include <nlohmann/json.hpp>
#include <iomanip>
#include <fstream>
#include "mock/mock.h"

/**
 *  ./mock.sh -n 1G -d "kvdict"
 */
using json = nlohmann::json;
static const std::string configFile = std::filesystem::path(PROJECT_DIR) / "config.json";

static json &getConfig()
{
    static json instance;
    return instance;
}

static void loadConfig(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file)
    {
        LOG_ERROR("Failed to open config file: " + filePath);
        throw std::runtime_error("Failed to open config file: " + filePath);
    }
    file >> getConfig(); // 把数据读入静态 json 对象
}

class MockCmd
{
public:
    std::string directory = "kvdict"; // 默认目录是 "kvdict"
    std::string keyPrefix;
    std::string valuePrefix;
    double maxFileSizeMB;
    double maxSizeGB;
    double targetSizeMB;
    double approxEntrySizeKB;

    MockCmd()
    {
        try
        {
            // 加载配置文件
            loadConfig(configFile);
            if (getConfig().contains("targetSizeMB"))
            {
                targetSizeMB = getConfig()["targetSizeMB"].get<double>();
            }
            if (getConfig().contains("keyPrefix"))
            {
                keyPrefix = getConfig()["keyPrefix"].get<std::string>();
            }
            if (getConfig().contains("valuePrefix"))
            {
                valuePrefix = getConfig()["valuePrefix"].get<std::string>();
            }
            if (getConfig().contains("maxFileSizeMB"))
            {
                maxFileSizeMB = getConfig()["maxFileSizeMB"].get<double>();
            }
            if (getConfig().contains("maxSizeGB"))
            {
                maxSizeGB = getConfig()["maxSizeGB"].get<double>();
            }
            if (getConfig().contains("approxEntrySizeKB"))
            {
                approxEntrySizeKB = getConfig()["approxEntrySizeKB"].get<double>();
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Failed to load configuration: " + std::string(e.what()));
        }
    }

    Result parse(int argc, char **argv)
    {
        int opt;
        while ((opt = getopt(argc, argv, "n:d:")) != -1)
        {
            switch (opt)
            {
            case 'n':
                targetSizeMB = stod(parseSize(optarg).message_raw()); // 解析 -n 后的值
                break;
            case 'd':
                directory = optarg; // 解析 -d 后的值
                break;
            default:
                LOG_INFO("Usage: ./mock -n <size> -d <directory>");
                LOG_INFO("Example: ./mock -n 1G -d kvdict");
                return Result(Result::kError, "Invalid option");
            }
        }
        return Result(Result::kOk, "Parsed successfully");
    }

private:
    // 解析像 10G 这种带有单位的大小参数
    Result parseSize(const char *sizeStr)
    {
        size_t size = 0;
        std::string str(sizeStr);
        LOG_DEBUG("Parsing target size: " + str);
        char unit = str.back(); // 获取最后的单位字符（G 或 M）

        // 去掉最后的字符（单位）
        str.pop_back();

        try
        {
            size = stod(str); // 将剩下的部分转换为数字
        }
        catch (const std::exception &e)
        {
            return Result(Result::kError, "Invalid size format: " + std::string(sizeStr));
        }

        LOG_DEBUG("Parsed size: " + std::to_string(size) + " with unit: " + unit);

        if (unit == 'G')
        {
            return Result(Result::kOk, std::to_string(size * 1024)); // 转成 MB
        }
        else if (unit == 'M')
        {
            return Result(Result::kOk, std::to_string(size));
        }
        else
        {
            return Result(Result::kError, "Invalid size unit, only 'G' or 'M' are supported");
        }
    }
};

int main(int argc, char **argv)
{
    MockCmd cmd;
    Result res = cmd.parse(argc, argv);
    if (res.isError())
    {
        LOG_ERROR("Failed to parse command line arguments: " + res.message());
        return 1;
    }

    LOG_DEBUG("Target size: " + std::to_string(cmd.targetSizeMB) + "MB");
    LOG_DEBUG("Directory: " + cmd.directory);
    LOG_DEBUG("Approximate entry size: " + std::to_string(cmd.approxEntrySizeKB) + "KB");

    // 检查目录是否存在
    json folderHistory = LoadFolderHistory();
    LOG_DEBUG("Loaded folder history: " + folderHistory.dump());

    if (!CheckFolderNameUnique(folderHistory, cmd.directory))
    {
        LOG_ERROR("Folder name conflict detected: " + cmd.directory);
        return -1;
    }
    LOG_DEBUG("Folder name is unique: " + cmd.directory);

    if (!SaveFolderHistory(folderHistory, cmd.directory))
    {
        LOG_ERROR("Failed to save folder history.");
        return -1;
    }

    LOG_DEBUG("Folder history saved successfully.");

    try
    {
        std::ofstream ofs(configFile);
        if (!ofs.is_open())
        {
            LOG_ERROR("Failed to open config file: " + configFile);
            return -1;
        }

        nlohmann::json config = {
            {"keyPrefix", cmd.keyPrefix},
            {"valuePrefix", cmd.valuePrefix},
            {"maxFileSizeMB", cmd.maxFileSizeMB},
            {"maxSizeGB", cmd.maxSizeGB},
            {"targetSizeMB", cmd.targetSizeMB},
            {"approxEntrySizeKB", cmd.approxEntrySizeKB}};

        ofs << config.dump(4); // 格式化输出
        ofs.close();

        if (!ofs)
        {
            LOG_ERROR("Write failed");
            return -1;
        }
        LOG_DEBUG("Wrote config to " + configFile);

        // 构造并启动数据生成器
        DataGen generator(configFile, cmd.directory);
        LOG_DEBUG("Starting data generation...");
        generator.generateData();
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Data generation failed: " + std::string(e.what()));
        return -1;
    }

    return 0;
}