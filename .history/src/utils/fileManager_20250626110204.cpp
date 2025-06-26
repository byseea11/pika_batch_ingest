#include "fileManager.h"
#include "log.h"
#include <json/value.h>
Result FileManager::write(const std::vector<std::unordered_map<std::string, std::string>> &data)
{
    Json::Value root;
    for (const auto &item : data)
    {
        root.append(item); // 假设数据是一个整数向量
    }

    getFileName();
    std::ofstream file(filePath_);
    if (!file.is_open())
    {
        LOG_ERROR("Failed to open file for writing: " + filePath_);
        return Result(Result::Ret::kFileOpenError, filePath_);
    }

    file << root.dump(4);
    file.close();
    return Result(Result::Ret::kOk, filePath_);
}