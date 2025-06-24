#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <random>

// 生成指定长度的随机字符串
std::string generate_random_string(int length)
{
    const std::string characters = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string random_string;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);

    for (int i = 0; i < length; ++i)
    {
        random_string += characters[distribution(generator)];
    }

    return random_string;
}

// 生成一个随机整数
int generate_random(int start, int end)
{
    return start + rand() % (end - start + 1);
}

// 生成随机的键名和值
std::string generate_command_from_config(const std::string &config_path, int i)
{
    std::ostringstream oss;

    // 随机生成键名后缀和键值后缀
    std::string key_suffix = generate_random_string(8);   // 随机生成 8 位字符串
    std::string value_suffix = generate_random_string(8); // 随机生成 8 位字符串

    // 生成键名和键值
    std::string key = "key_" + key_suffix;
    std::string value = "value_" + value_suffix;

    // 生成随机值或步进值
    int step = 0;
    bool random_flag = true;
    if (random_flag)
    {
        step = generate_random(1, 100); // 随机生成步进值
    }

    // 生成命令（例如 SET）
    oss << "SET " << key << " " << value;

    return oss.str();
}