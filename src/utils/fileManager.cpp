#include "fileManager.h"
Result FileManager::write(const DataGen *gen)
{
    Json::Value root;
    for (const auto& item : data) {
        root.append(item);  // 假设数据是一个整数向量
    }

    getNextFile();
    std::ofstream file(filePath_);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file for writing: " + filePath_);
        return Result(Result::Ret::kFileOpenError, filePath_);
    }

    file << root.dump(4);
    file.close();
    return Result(Result::Ret::kOk, filePath_);
}