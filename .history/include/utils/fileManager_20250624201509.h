#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <fstream>
#include <string>
#include <filesystem>
#include "result.h"
#include "dataGen.h"

class FileManager
{
public:
    FileManager() = default;
    // 构造函数，接受文件路径
    FileManager(const std::string &filePath) : file_(filePath, std::ios::out | std::ios::app), filePath_(filePath), result_(Result(Result::Ret::kOk, filePath))
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
        filePath_ = "";
    }
    // 写入数据到文件，这里需要先mock，这个函数应该是传入一个mock对象，和需要生成的数据大小，然后处理逻辑
    Result write(const DataGen gen, const size_t size);

    // 获取文件大小
    size_t getFileSize()
    {
        if (file_.is_open())
        {
            file_.seekp(0, std::ios::end);
            return file_.tellp();
        }
        return 0;
    }

private:
    std::ofstream file_;
    std::string filePath_; // 文件路径
    Result result_;        // 用于管理文件操作的状态
};
#endif