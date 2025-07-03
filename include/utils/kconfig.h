

#ifndef KCONFIG_H
#define KCONFIG_H
#include <filesystem>
#ifndef PROJECT_DIR
#define PROJECT_DIR "."
#endif

const std::filesystem::path DEFAULTDIC = std::filesystem::path(PROJECT_DIR) / "mock";
const std::filesystem::path DEFAULTCONFIGFILEDIC = std::filesystem::path(PROJECT_DIR) / "dics.json";

#endif