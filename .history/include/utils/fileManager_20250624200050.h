#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <fstream>
#include <string>
#include <filesystem>
#include "result.h"

class FileManager
{
public:
    FileManager() = default;
    // 构造函数，接受文件路径
    FileManager(const std::string &filePath) : file_(filePath, std::ios::out | std::ios::app), result_(new Result(Result::Ret::kOk))
    {
        if (!file_.is_open())
        {
            result_.setRes(Result::kFileOpenError);
        }
    }

private:
    std::ofstream file_;
    Result result_; // 用于管理文件操作的状态
};
#endif