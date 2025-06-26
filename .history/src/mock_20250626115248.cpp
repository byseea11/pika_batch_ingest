#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <stdexcept>
#include "log.h"
#include "dataGen.h"

class MockCmd
{
public:
    size_t targetSizeGB = 1;          // 默认值是 1G
    std::string directory = "kvdata"; // 默认目录是 "kvdata"

    void parse(int argc, char **argv)
    {
        int opt;
        while ((opt = getopt(argc, argv, "n:d:")) != -1)
        {
            switch (opt)
            {
            case 'n':
                targetSizeGB = parseSize(optarg); // 解析 -n 后的值
                break;
            case 'd':
                directory = optarg; // 解析 -d 后的值
                break;
            default:
                throw std::invalid_argument("Invalid option");
            }
        }
    }

private:
    // 解析像 10G 这种带有单位的大小参数
    size_t parseSize(const char *sizeStr)
    {
        size_t size = 0;
        std::string str(sizeStr);
        char unit = str.back(); // 获取最后的单位字符（G 或 M）

        // 去掉最后的字符（单位）
        str.pop_back();

        try
        {
            size = std::stoul(str); // 将剩下的部分转换为数字
        }
        catch (const std::exception &e)
        {
            throw std::invalid_argument("Invalid size format");
        }

        if (unit == 'G')
        {
            return size; // 以 GB 为单位
        }
        else if (unit == 'M')
        {
            return size / 1024; // 将 MB 转换为 GB
        }
        else
        {
            throw std::invalid_argument("Invalid size unit, only 'G' or 'M' are supported");
        }
    }
};

int main(int argc, char **argv)
{
    MockCmd cmd;
    try
    {
        cmd.parse(argc, argv);
        LOG_INFO("Target size: " + std::to_string(cmd.targetSizeGB) + "G");
        LOG_INFO("Directory: " + cmd.directory);
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Error parsing arguments: " + std::string(e.what()));
        return 1;
    }

    try
    {
        // 检查并创建目录
        if (!std::filesystem::exists(cmd.directory))
        {
            std::filesystem::create_directories(cmd.directory);
            LOG_INFO("Created directory: " + cmd.directory);
        }

        // 构造配置文件路径
        std::string configFile = "uconfig.json";

        // 将命令行参数写入配置文件（如果你希望动态生成配置）
        std::ofstream ofs(configFile);
        if (ofs.is_open())
        {
            ofs << R"({
                "keyPrefix": "key_",
                "valuePrefix": "value_",
                "maxFileSizeMB": 256,
                "targetSizeGB": )"
                << config.targetSizeGB << R"(,
                "maxSizeGB": 100
            })";
            ofs.close();
            LOG_INFO("Wrote config to " + configFile);
        }
        else
        {
            LOG_ERROR("Failed to write configuration file.");
            return 1;
        }

        // 构造并启动数据生成器
        DataGen generator(configFile, config.directory);
        generator.generateData();
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Data generation failed: " + std::string(e.what()));
        return 1;
    }

    return 0;
}