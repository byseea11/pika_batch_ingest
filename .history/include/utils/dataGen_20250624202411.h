#ifndef DATAGEN_H
#define DATAGEN_H
#include <string>
#include <random>
#include <fstream>
#include "fileManager.h"
#include "json.hpp"

using json = nlohmann::json;

class DataGen
{

private:
    FileManager &fileManager_;
    json config_;
    std::default_random_engine generator_;
    std::uniform_int_distribution<int> dist_;
};

#endif