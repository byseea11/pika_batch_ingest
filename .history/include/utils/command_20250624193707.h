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
            return "No operation performed.";
        case kOk:
            return "Operation completed successfully.";
        case kFileCreated:
            return "File created successfully.";
        case kDataGenerated:
            return "Data generated successfully.";
        case kError:
            return "An unknown error occurred.";
        case kInvalidSize:
            return "Invalid size parameter.";
        case kInvalidParam:
            return "Invalid parameters provided.";
        case kFileWriteError:
            return "Error writing to file.";
        case kOutOfMemory:
            return "Out of memory.";
        case kFileTooLarge:
            return "File size exceeds the allowed limit.";
        case kInvalidUnit:
            return "Invalid unit (use B, K, M, G).";
        case kInvalidRange:
            return "The specified range is invalid.";
        case kInvalidFileType:
            return "Invalid file type (only .json supported).";
        case kDataSizeMismatch:
            return "The generated data size does not match the expected size.";
        case kProcessing:
            return "Data generation in progress.";
        case kCancelled:
            return "Operation was cancelled.";
        default:
            return "Unknown status.";
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
