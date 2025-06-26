#include "fileManager.h"
#include "log.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Result FileManager::write(const std::vector<std::unordered_map<std::string, std::string>> &data)
{
    getFileName();

    std::ofstream file(filePath_);
    if (!file.is_open())
    {
        LOG_ERROR("Failed to open file for writing: " + filePath_);
        return Result(Result::Ret::kFileOpenError, filePath_);
    }

    // 自动将 vector<unordered_map<string, string>> 转为 json array of objects
    json j = data;
    file << j.dump(4); // 缩进为 4 空格，美化输出
    file.close();

    return Result(Result::Ret::kOk, filePath_);
}