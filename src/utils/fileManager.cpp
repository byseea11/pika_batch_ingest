#include "mock/fileManager.h"
#include "utils/klog.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Result FileManager::write(const DataType &data)
{
    getFileName();
    LOG_DEBUG("Writing data to file: " + filePath_);

    std::ofstream file(filePath_);
    if (!file.is_open())
    {
        LOG_ERROR("Failed to open file for writing: " + filePath_);
        return Result(Result::Ret::kFileOpenError, filePath_);
    }

    json j = data;
    file << j.dump(4); // 缩进为 4 空格，美化输出
    file.close();

    return Result(Result::Ret::kOk, filePath_);
}