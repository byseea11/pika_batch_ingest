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

namespace fs = std::filesystem;

enum class StringCommand
{
    SET,
    GET,
    INCRBY,
    DECRBY,
    PSETEX,
    STRLEN,
    APPEND,
    BITOP,
    SETBIT,
    GETBIT,
    UNKNOWN
};

StringCommand parse_command(const std::string &cmd)
{
    static const std::unordered_map<std::string, StringCommand> mapping = {{"SET", String::SET},
                                                                           {"GET", StringCommand::GET},
                                                                           {"INCRBY", StringCommand::INCRBY},
                                                                           {"DECRBY", StringCommand::DECRBY},
                                                                           {"PSETEX", StringCommand::PSETEX},
                                                                           {"STRLEN", StringCommand::STRLEN},
                                                                           {"APPEND", StringCommand::APPEND},
                                                                           {"BITOP", StringCommand::BITOP},
                                                                           {"SETBIT", StringCommand::SETBIT},
                                                                           {"GETBIT", StringCommand::GETBIT}};
    auto it = mapping.find(cmd);
    return it != mapping.end() ? it->second : StringCommand::UNKNOWN;
}

bool is_partial_supported(StringCommand cmd)
{
    return cmd == StringCommand::BITOP || cmd == StringCommand::SETBIT || cmd == StringCommand::GETBIT;
}

std::string generate_supported_command(StringCommand cmd, int i)
{
    std::ostringstream oss;
    std::string key = "key_" + std::to_string(i);
    std::string value = "value_" + std::to_string(i);

    switch (cmd)
    {
    case StringCommand::SET:
        oss << "SET " << key << " " << value;
        break;
    case StringCommand::GET:
        oss << "GET " << key;
        break;
    case StringCommand::INCRBY:
        oss << "INCRBY " << key << " " << (i % 100);
        break;
    case StringCommand::DECRBY:
        oss << "DECRBY " << key << " " << (i % 50);
        break;
    case StringCommand::PSETEX:
        oss << "PSETEX " << key << " 60000 " << value;
        break;
    case StringCommand::STRLEN:
        oss << "STRLEN " << key;
        break;
    case StringCommand::APPEND:
        oss << "APPEND " << key << " _suffix";
        break;
    default:
        break;
    }
    return oss.str();
}

std::string generate_partial_supported_command(StringCommand cmd, int i)
{
    std::ostringstream oss;
    std::string key = "key_" + std::to_string(i);
    switch (cmd)
    {
    case StringCommand::BITOP:
        oss << "# WARNING: BITOP command behavior differs from Redis\n";
        oss << "BITOP AND dest_key " << key << " another_key";
        break;
    case StringCommand::SETBIT:
        oss << "# WARNING: SETBIT may trigger RocksDB write amplification\n";
        oss << "SETBIT " << key << " 7 1";
        break;
    case StringCommand::GETBIT:
        oss << "# WARNING: GETBIT may behave differently\n";
        oss << "GETBIT " << key << " 7";
        break;
    default:
        break;
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
            std::string line =
                is_partial_supported(cmd) ? generate_partial_supported_command(cmd, i) : generate_supported_command(cmd, i);
            out << line << "\n";
        }
    }

    out.close();
    std::cout << "Mock data written to " << output_path << " (" << count << " records per command)" << std::endl;
    return 0;
}
