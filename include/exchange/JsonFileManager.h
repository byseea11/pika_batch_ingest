#ifndef JSONFILEMANAGER_H
#define JSONFILEMANAGER_H
#include <fstream>
#include <nlohmann/json.hpp>
#include "utils/kvEntry.h"
using json = nlohmann::json;

// 使用虚拟函数来支持 Mock
class JsonFileManagerBase
{
public:
    virtual ~JsonFileManagerBase() = default;

    // 解析 JSON 文件的纯虚函数
    virtual DataType parse(const std::string &jsonStr) = 0; // 改为虚函数

protected:
    JsonFileManagerBase() = default;
};

class JsonFileManager : public JsonFileManagerBase
{
public:
    DataType parse(const std::string &filePath)
    {
        json j = load(filePath);
        KvData data;
        for (const auto &item : j)
        {
            if (item.is_object() && item.contains("key") && item.contains("value"))
            {
                KvEntry entry;
                entry.key = item["key"].get<std::string>();
                entry.value = item["value"].get<std::string>();
                if (item.contains("expire"))
                {
                    entry.timestamp = item["expire"].get<uint32_t>();
                }
                else
                {
                    entry.timestamp = 0;
                }
                data.push_back(entry);
            }
            else
            {
                throw std::runtime_error("Invalid JSON format");
            }
        }
        return data;
    };

    json load(const std::string &filePath)
    {
        std::ifstream in(filePath);
        if (!in)
            throw std::runtime_error("File open failed: " + filePath);
        json j;
        in >> j;
        return j;
    }
};

#endif // JSONFILEMANAGER_H