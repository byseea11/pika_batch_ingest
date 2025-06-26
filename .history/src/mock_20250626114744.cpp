#include <iostream>
#include <getopt.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <stdexcept>
#include "log.h"

class MockConfig
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
    MockConfig config;
    try
    {
        config.parse(argc, argv);
        LOG_INFO("Target size: " + std::to_string(config.targetSizeGB) + "G");
        LOG_INFO("Directory: " + config.directory);
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Error parsing arguments: " + std::string(e.what()));
        return 1;
    }

    // 根据解析到的 config 变量来执行相应操作
    // 比如根据 targetSizeGB 和 directory 来生成数据文件等...

    return 0;
}
