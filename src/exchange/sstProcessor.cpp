#include "exchange/sstProcessor.h"
#include "exchange/JsonFileManager.h"
#include "utils/result.h"
#include <iostream>

Result SstProcessor::processSstFile(const std::string &inputJsonPath, const std::string &outputSstPath)
{
    JsonFileManager fileManager;
    DataType data;

    try
    {
        data = fileManager.parse(inputJsonPath);
    }
    catch (const std::exception &e)
    {
        return Result(Result::Ret::kFileReadError, std::string("JSON parse failed: ") + e.what());
    }

    rocksdb::SstFileWriter writer(rocksdb::EnvOptions(), options_, cfh_);

    auto status = writer.Open(outputSstPath);
    if (!status.ok())
    {
        return Result(Result::Ret::kFileWriteError, "Failed to open SST file: " + status.ToString());
    }

    for (const auto &entry : data)
    {
        status = writer.Put(entry.key, entry.encodedValue());
        if (!status.ok())
        {
            writer.Finish().PermitUncheckedError();
            return Result(Result::Ret::kFileWriteError, "Put failed: " + status.ToString());
        }
    }

    status = writer.Finish();
    if (!status.ok())
    {
        return Result(Result::Ret::kFileWriteError, "Finish failed: " + status.ToString());
    }

    return Result(Result::Ret::kOk, "SST file created successfully: " + outputSstPath);
}
