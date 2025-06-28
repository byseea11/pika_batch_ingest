#include <iostream>
#include <string>
#include <unistd.h>
#include "exchange/sstProcessor.h"
#include "exchange/JsonFileManager.h"

void print_usage(const char *prog)
{
    std::cout << "Usage: " << prog << " -k <kvPath> -s <sst_path>\n";
}

int main(int argc, char **argv)
{
    std::string kvPath;
    std::string sstPath;

    int opt;
    while ((opt = getopt(argc, argv, "k:s:")) != -1)
    {
        switch (opt)
        {
        case 'k':
            kvPath = optarg;
            break;
        case 's':
            sstPath = optarg;
            break;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    if (kvPath.empty() || sstPath.empty())
    {
        print_usage(argv[0]);
        return 1;
    }

    // 打印参数
    std::cout << "Read JSON file: " << kvPath << " to SST: " << sstPath << std::endl;

    // 创建 JsonFileManager
    JsonFileManager fileManager;

    // 创建 SstProcessor
    rocksdb::Options options;
    options.create_if_missing = true;

    SstProcessor processor(options);

    // 调用
    Result result = processor.processSstFile(&fileManager, kvPath, sstPath);
    if (result.getRet() == Result::Ret::kOk)
    {
        std::cout << "Success: " << result.message() << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "Error: " << result.message() << std::endl;
        return 1;
    }
}
