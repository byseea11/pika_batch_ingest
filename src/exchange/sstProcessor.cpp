#include "exchange/sstProcessor.h"
#include "exchange/JsonFileManager.h"
#include "utils/compare.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

Result SstProcessor::processSstFile(JsonFileManagerBase *fileManager,
                                    const std::string &inputJsonPath,
                                    const std::string &outputSstPath)
{
    DataType data;
    std::string ac_inputJsonPath = DEFAULTDIC / inputJsonPath;
    std::string ac_outputSstPath = DEFAULTDIC / outputSstPath;
    try
    {
        data = fileManager->parse(ac_inputJsonPath); // 使用传入的 fileManager 进行解析
    }
    catch (const std::exception &e)
    {
        return Result(Result::Ret::kFileReadError, "JSON parse failed: " + std::string(e.what()));
    }

    // 确保输出路径的父目录存在
    try
    {
        fs::path outputPath(ac_outputSstPath);
        fs::path parentDir = outputPath.parent_path();

        if (!parentDir.empty() && !fs::exists(parentDir))
        {
            std::cout << "Creating directory: " << parentDir << std::endl;
            fs::create_directories(parentDir);
        }
    }
    catch (const std::exception &e)
    {
        return Result(Result::Ret::kFileWriteError, "Failed to create directory: " + std::string(e.what()));
    }

    // 创建 SstFileWriter
    rocksdb::SstFileWriter writer(rocksdb::EnvOptions(), options_, cfh_);

    auto status = writer.Open(ac_outputSstPath);
    if (!status.ok())
    {
        return Result(Result::Ret::kFileWriteError, "Failed to open SST file: " + status.ToString());
    }

    // 先按 ComparePair 排序
    std::sort(data.begin(), data.end(), ComparePair());

    // 去重处理
    std::vector<KvEntry> deduped;
    for (size_t i = 0; i < data.size();)
    {
        deduped.push_back(data[i]);
        // skip all with same key
        size_t j = i + 1;
        while (j < data.size() && data[j].key == data[i].key)
        {
            ++j;
        }
        i = j;
    }

    // 写入去重后的数据
    for (const auto &entry : deduped)
    {
        status = writer.Put(entry.key, entry.encodedValue());
        if (!status.ok())
        {
            writer.Finish().PermitUncheckedError();
            return Result(Result::Ret::kFileWriteError, "Put failed: " + status.ToString());
        }
    }

    // 完成 SST 写入
    status = writer.Finish();
    if (!status.ok())
    {
        return Result(Result::Ret::kFileWriteError, "Finish failed: " + status.ToString());
    }

    return Result(Result::Ret::kOk, "SST file created successfully: " + ac_outputSstPath);
}
