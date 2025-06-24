#ifndef COMMAND_H
#define COMMAND_H
#include <string>

class Result
{
public:
    enum CmdRet
    {
        kNone = 0,         // 默认状态
        kOk,               // 成功
        kFileCreated,      // 文件创建成功
        kDataGenerated,    // 数据成功生成
        kError,            // 一般错误
        kInvalidSize,      // 无效的文件大小
        kInvalidParam,     // 无效的参数
        kFileWriteError,   // 文件写入错误
        kOutOfMemory,      // 内存不足
        kFileTooLarge,     // 文件太大
        kInvalidUnit,      // 无效的单位（B, K, M, G）
        kInvalidRange,     // 超出允许的范围
        kInvalidFileType,  // 无效的文件类型
        kDataSizeMismatch, // 数据大小不匹配
        kProcessing,       // 数据生成中
        kCancelled         // 操作被取消
    };

    // 返回状态和消息
    std::string message() const
    {
        switch (ret_)
        {
        case kNone:
            return "-ERR No operation performed\r\n";
        case kOk:
            return "+OK\r\n";
        case kFileCreated:
            return "+OK File created successfully\r\n";
        case kDataGenerated:
            return "+OK Data generated successfully\r\n";
        case kError:
            return "-ERR An unknown error occurred\r\n";
        case kInvalidSize:
            return "-ERR Invalid size parameter\r\n";
        case kInvalidParam:
            return "-ERR Invalid parameters provided\r\n";
        case kFileWriteError:
            return "-ERR Error writing to file\r\n";
        case kOutOfMemory:
            return "-ERR Out of memory\r\n";
        case kFileTooLarge:
            return "-ERR File size exceeds the allowed limit\r\n";
        case kInvalidUnit:
            return "-ERR Invalid unit (use B, K, M, G)\r\n";
        case kInvalidRange:
            return "-ERR The specified range is invalid\r\n";
        case kInvalidFileType:
            return "-ERR Invalid file type (only .json supported)\r\n";
        case kDataSizeMismatch:
            return "-ERR The generated data size does not match the expected size\r\n";
        case kProcessing:
            return "-ERR Data generation in progress\r\n";
        case kCancelled:
            return "-ERR Operation was cancelled\r\n";
        default:
            return "-ERR Unknown status\r\n";
        }
    }

    // 设置状态和消息
    void setRes(CmdRet ret, const std::string &content = "")
    {
        ret_ = ret;
        message_ = content;
    }

private:
    CmdRet ret_ = kNone;
    std::string message_;
};

#endif
