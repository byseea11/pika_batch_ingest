/**
 * @file     tools/mock.cpp
 * @brief    模拟 Pika 中 string 类型已实现命令接口的行为，用于大规模数据生成与验证
 *
 * @details
 * 本工具覆盖 Pika 支持的 string 类型命令，模拟完整的 CRUD 操作（创建、读取、更新、删除），
 * 可用于大规模批量写入前的数据准备。生成的 mock 数据将以文本格式输出（如 txt 文件），
 * 后续可转换为 RocksDB 的 SST 文件并注入到 Pika，绕过逐条 Redis 命令写入的低效路径，
 * 实现高效数据导入。
 *
 * 支持的命令接口如下：
 * - 完全支持：
 *   SET, GET, INCRBY, DECRBY, PSETEX, STRLEN, APPEND
 *
 * - 功能支持但行为与 Redis 略有差异：
 *   BITOP, SETBIT, GETBIT
 *
 * - 当前未支持的接口：
 *   BITFIELD
 *
 * 支持的命令行参数：
 * @param -n <int>   生成记录条数，默认值为 1,000,000
 * @param -p <path>  输出 mock 文本数据的路径
 * @param -t <str>   模拟数据类型，目前仅支持 string（保留扩展）
 * @param -c <str>   指定需要生成的命令，如 SET,GET,INCRBY ，多个用逗号分隔
 *
 * @example
 *  ```bash
 *  ./mock -n 100000 -p ./out.txt -t string -c SET,GET,SETBIT
 *  ```
 *
 * @note
 * BIT 操作由于 RocksDB 的写放大机制存在性能隐患，因此与 Redis 行为略有差异，详见文档记录。
 */

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;
static const std::string config_path = "config.json"; // 配置文件路径

enum class StringCommand
{
    APPEND,
    BITCOUNT,
    BITPOS,
    DECR,
    DECRBY,
    GET,
    GETRANGE,
    GETSET,
    INCR,
    INCRBY,
    INCRBYFLOAT,
    MGET,
    MSET,
    MSETNX,
    STRLEN,
    PSETEX,
    SET,
    SETEX,
    SETNX,
    SETRANGE,
    // 输出与redis有差异
    BITOP,
    GETBIT,
    SETBIT,
    UNKNOWN
};

static const std::unordered_map<std::string, StringCommand> mapping = {
    {"SET", StringCommand::SET},
    {"GET", StringCommand::GET},
    {"INCRBY", StringCommand::INCRBY},
    {"DECRBY", StringCommand::DECRBY},
    {"PSETEX", StringCommand::PSETEX},
    {"STRLEN", StringCommand::STRLEN},
    {"APPEND", StringCommand::APPEND},
    {"BITOP", StringCommand::BITOP},
    {"SETBIT", StringCommand::SETBIT},
    {"GETBIT", StringCommand::GETBIT},
    {"BITCOUNT", StringCommand::BITCOUNT},
    {"BITPOS", StringCommand::BITPOS},
    {"DECR", StringCommand::DECR},
    {"INCR", StringCommand::INCR},
    {"INCRBYFLOAT", StringCommand::INCRBYFLOAT},
    {"MGET", StringCommand::MGET},
    {"MSET", StringCommand::MSET},
    {"MSETNX", StringCommand::MSETNX},
    {"GETRANGE", StringCommand::GETRANGE},
    {"GETSET", StringCommand::GETSET},
    {"SETEX", StringCommand::SETEX},
    {"SETNX", StringCommand::SETNX},
    {"SETRANGE", StringCommand::SETRANGE}};

StringCommand parse_command(const std::string &cmd)
{
    auto it = mapping.find(cmd);
    return it != mapping.end() ? it->second : StringCommand::UNKNOWN;
}

bool is_partial_supported(StringCommand cmd)
{
    return cmd == StringCommand::BITOP || cmd == StringCommand::SETBIT || cmd == StringCommand::GETBIT;
}

/**
 * @brief 从配置文件读取并生成 Redis 命令
 *
 * 通过读取 JSON 配置文件，动态生成 Redis 命令字符串。
 *
 * @param config_path 配置文件路径
 * @param i 用于生成动态键名和值的索引
 * @return std::string 生成的命令
 */
std::string generate_command_from_config(StringCommand cmd, int i)
{
    // 读取配置文件
    std::ifstream config_file(config_path);
    json config;
    config_file >> config;

    std::ostringstream oss;

    // 遍历每个命令配置
    for (const auto &cmd_config : config["commands"])
    {
        std::string command = cmd_config["command"];
        std::string key = cmd_config["key_prefix"].get<std::string>() + std::to_string(i);
        std::string value = cmd_config["value_prefix"].get<std::string>() + std::to_string(i);

        if (command == "SET" && cmd == StringCommand::SET)
        {
            oss << "SET " << key << " " << value;
        }
        else if (command == "INCRBY" && cmd == StringCommand::INCRBY)
        {
            int increment = cmd_config["range_start"].get<int>() +
                            (i % (cmd_config["range_end"].get<int>() - cmd_config["range_start"].get<int>()));
            oss << "INCRBY " << key << " " << increment;
        }
        else if (command == "DECRBY" && cmd == StringCommand::INCRBY)
        {
            int decrement = cmd_config["range_start"].get<int>() +
                            (i % (cmd_config["range_end"].get<int>() - cmd_config["range_start"].get<int>()));
            oss << "DECRBY " << key << " " << decrement;
        }
        else if (command == "PSETEX" && cmd == StringCommand::PSETEX)
        {
            int milliseconds = cmd_config["range_start"].get<int>() +
                               (i % (cmd_config["range_end"].get<int>() - cmd_config["range_start"].get<int>()));
            oss << "PSETEX " << key << " " << milliseconds << " " << value;
        }
        else if (command == "STRLEN" && cmd == StringCommand::STRLEN)
        {
            oss << "STRLEN " << key;
        }
        else if (command == "APPEND" && cmd == StringCommand::APPEND)
        {
            oss << "APPEND " << key << " _suffix_" << i;
        }
        else if (command == "BITCOUNT" && cmd == StringCommand::BITCOUNT)
        {
            oss << "BITCOUNT " << key;
            if (!cmd_config["extra_params"].empty())
            {
                oss << " " << cmd_config["extra_params"][0] << " " << cmd_config["extra_params"][1];
            }
        }
        else if (command == "BITPOS" && cmd == StringCommand::BITPOS)
        {
            oss << "BITPOS " << key << " " << cmd_config["extra_params"][0];
            if (cmd_config["extra_params"].size() > 1)
            {
                oss << " " << cmd_config["extra_params"][1] << " " << cmd_config["extra_params"][2];
            }
        }
        else if (command == "DECR" && cmd == StringCommand::DECR)
        {
            oss << "DECR " << key;
        }
        else if (command == "INCR" && cmd == StringCommand::INCR)
        {
            oss << "INCR " << key;
        }
        else if (command == "INCRBYFLOAT" && cmd == StringCommand::INCRBYFLOAT)
        {
            float increment = cmd_config["range_start"].get<float>() +
                              (i % (cmd_config["range_end"].get<int>() - cmd_config["range_start"].get<int>()));
            oss << "INCRBYFLOAT " << key << " " << increment;
        }
        else if (command == "MGET" && cmd == StringCommand::MGET)
        {
            oss << "MGET";
            for (const auto &param : cmd_config["extra_params"])
            {
                oss << " " << param;
            }
        }
        else if (command == "MSET" && cmd == StringCommand::MSET)
        {
            oss << "MSET";
            for (size_t j = 0; j < cmd_config["extra_params"].size(); j += 2)
            {
                oss << " " << cmd_config["extra_params"][j] << " " << cmd_config["extra_params"][j + 1];
            }
        }
        else if (command == "MSETNX" && cmd == StringCommand::MSETNX)
        {
            oss << "MSETNX";
            for (size_t j = 0; j < cmd_config["extra_params"].size(); j += 2)
            {
                oss << " " << cmd_config["extra_params"][j] << " " << cmd_config["extra_params"][j + 1];
            }
        }
        else if (command == "SETRANGE" && cmd == StringCommand::SETRANGE)
        {
            oss << "SETRANGE " << key << " " << cmd_config["extra_params"][0] << " " << value;
        }
        else if (command == "SETEX" && cmd == StringCommand::SETEX)
        {
            int seconds = cmd_config["range_start"].get<int>() +
                          (i % (cmd_config["range_end"].get<int>() - cmd_config["range_start"].get<int>()));
            oss << "SETEX " << key << " " << seconds << " " << value;
        }
        else if (command == "SETNX" && cmd == StringCommand::SETNX)
        {
            oss << "SETNX " << key << " " << value;
        }
        else if (command == "GETRANGE" && cmd == StringCommand::GETRANGE)
        {
            oss << "GETRANGE " << key << " " << cmd_config["extra_params"][0] << " " << cmd_config["extra_params"][1];
        }
        else if (command == "GETSET" && cmd == StringCommand::GETSET)
        {
            oss << "GETSET " << key << " " << value;
        }
        else if (cmd_config["command"] == "BITOP" && cmd == StringCommand::BITOP)
        {
            oss << "BITOP " << cmd_config["operation"] << " "
                << cmd_config["dest_key"] << " ";
            for (const auto &k : cmd_config["keys"])
            {
                oss << k << " ";
            }
            break;
        }
        else if (cmd_config["command"] == "SETBIT" && cmd == StringCommand::SETBIT)
        {
            oss << "SETBIT " << key << " "
                << cmd_config["offset"] << " " << cmd_config["value"];
            break;
        }
        else if (cmd_config["command"] == "GETBIT" && cmd == StringCommand::GETBIT)
        {
            oss << "GETBIT " << key << " " << cmd_config["offset"];
            break;
        }

        // 输出命令
        std::cout << oss.str() << std::endl;
    }

    return oss.str();
}

std::vector<std::string> split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delimiter))
    {
        tokens.push_back(item);
    }
    return tokens;
}

int main(int argc, char *argv[])
{
    int count = 1000000;
    std::string output_path = "./mock_out.txt";
    std::string type = "string";
    std::vector<StringCommand> commands;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-n" && i + 1 < argc)
        {
            count = std::atoi(argv[++i]);
        }
        else if (arg == "-p" && i + 1 < argc)
        {
            output_path = argv[++i];
        }
        else if (arg == "-t" && i + 1 < argc)
        {
            type = argv[++i];
        }
        else if (arg == "-c" && i + 1 < argc)
        {
            std::vector<std::string> raw_cmds = split(argv[++i], ',');
            for (const auto &cmd_str : raw_cmds)
            {
                StringCommand cmd = parse_command(cmd_str);
                if (cmd != StringCommand::UNKNOWN)
                    commands.push_back(cmd);
            }
        }
    }

    if (type != "string")
    {
        std::cerr << "Only string type is supported currently." << std::endl;
        return 1;
    }

    if (commands.empty())
    {
        std::cerr << "No valid commands specified with -c option." << std::endl;
        return 1;
    }

    fs::create_directories(fs::path(output_path).parent_path());
    std::ofstream out(output_path);
    if (!out.is_open())
    {
        std::cerr << "Failed to open output file: " << output_path << std::endl;
        return 1;
    }

    for (int i = 0; i < count; ++i)
    {
        for (const auto &cmd : commands)
        {
            std::string line = generate_command_from_config(cmd, i);
            out << line << "\n";
        }
    }

    out.close();
    std::cout << "Mock data written to " << output_path << " (" << count << " records per command)" << std::endl;
    return 0;
}
