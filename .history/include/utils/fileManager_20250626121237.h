#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <fstream>
#include <string>
#include <filesystem>
#include "utils/result.h"
#include <vector>
#include <unordered_map>
#include <mutex>

class FileManager
{
public:
    FileManager() = default;
    // 构造函数，接受文件路径
    FileManager(const std::string &dic) : dic_(dic), distname_index_(0) {}
    ~FileManager() {}

    // 线程安全的生成文件名
    Result getFileName()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        filePath_ = dic_ + "/data_" + std::to_string(distname_index_++) + ".json"; // 更新当前文件路径
        return Result(Result::Ret::kFileCreated, filePath_);
    }

    Result write(const std::vector<std::unordered_map<std::string, std::string>> &data);

    // 文件路径和扩展名验证
    bool validateFileExtension(const std::string &extension)
    {
        return filePath_.substr(filePath_.find_last_of('.') + 1) == extension;
    }

private:
    std::string dic_;                     // 用户指定的文件夹路径
    std::string filePath_;                // 当前文件的路径
    size_t distname_index_;               // 文件名的索引，用于生成唯一的文件名
    std::string fileExtension_ = ".json"; // 文件扩展名
    std::mutex mutex_;                    // 用于文件名分配的互斥锁
};

#endif