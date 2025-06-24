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

private:
    std::ofstream file_;
    Result result_; // 用于管理文件操作的状态
};
#endif