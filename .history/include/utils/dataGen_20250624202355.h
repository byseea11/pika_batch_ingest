#ifndef DATAGEN_H
#define DATAGEN_H
#include <string>
#include <random>
#include <fstream>
#include "FileManager.h"
#include "json.hpp"

using json = nlohmann::json;

class DataGen
{

private:
    std::string data;
};

#endif