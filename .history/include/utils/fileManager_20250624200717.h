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
    FileManager(const std::string &filePath) : file_(filePath, std::ios::out | std::ios::app), result_(Result(Result::Ret::kOk, filePath))
    {
        if (!file_.is_open())
        {
            result_.setRes(Result::kFileOpenError, filePath);
        }
    }
    // 析构函数，关闭文件
    ~FileManager()
    {
        if (file_.is_open())
        {
            file_.close();
        }
    }
    // 写入数据到文件
    Result write(const std::string &data)
    {
        if (!file_.is_open())
        {
            return Result(Result::Ret::kFileNotOpen, "File is not open");
        }
        file_ << data;
        if (file_.fail())
        {
            return Result(Result::Ret::kFileWriteError, "Failed to write to file");
        }
        return result_;
    }

private:
    std::ofstream file_;
    Result result_; // 用于管理文件操作的状态
};
#endif