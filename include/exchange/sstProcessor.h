#ifndef SST_PROCESSOR_H
#define SST_PROCESSOR_H

#include <string>
#include <memory>
#include "rocksdb/sst_file_writer.h"
#include "rocksdb/env.h"
#include "rocksdb/options.h"
#include "rocksdb/status.h"
#include "rocksdb/utilities/db_ttl.h"
#include "utils/result.h"
#include "exchange/JsonFileManager.h" // 包含 JsonFileManagerBase

class SstProcessor
{
public:
    // 构造函数：需要传入 ColumnFamilyHandle（或 nullptr 使用默认）
    SstProcessor(const rocksdb::Options &opts, rocksdb::ColumnFamilyHandle *cfh = nullptr)
        : options_(opts), cfh_(cfh) {}

    // 修改 processSstFile，允许注入 JsonFileManagerBase*
    Result processSstFile(JsonFileManagerBase *fileManager,
                          const std::string &inputJsonPath,
                          const std::string &outputSstPath);

    // 实现并发处理kv->sst:使用线程池  ThreadPool pool(numThreads_);多线程处理，读取input文件夹，每一个线程单独处理，可以使用processSstFile来处理，不过里面就需要加锁
    Result mutiProcessSstFile(JsonFileManagerBase *fileManager, const std::string &inputDicPath,
                              const std::string &outputDicPath);

private:
    rocksdb::Options options_;
    rocksdb::ColumnFamilyHandle *cfh_; // 可以为 nullptr 表示 default CF
};

#endif // SST_PROCESSOR_H

// 添加一个sst read的小工具