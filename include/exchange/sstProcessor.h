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
#include "exchange/JsonFileManager.h"

class SstProcessor
{
public:
    // 构造函数：需要传入 ColumnFamilyHandle（或 nullptr 使用默认）
    SstProcessor(const rocksdb::Options &opts, rocksdb::ColumnFamilyHandle *cfh = nullptr)
        : options_(opts), cfh_(cfh) {}

    Result processSstFile(const std::string &inputJsonPath, const std::string &outputSstPath);

private:
    rocksdb::Options options_;
    rocksdb::ColumnFamilyHandle *cfh_; // 可以为 nullptr 表示 default CF
};

#endif // SST_PROCESSOR_H
