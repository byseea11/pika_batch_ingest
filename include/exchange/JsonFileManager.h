#ifndef JSONFILEMANAGER_H
#define JSONFILEMANAGER_H
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class JsonFileManager
{
public:
    json load(const std::string &filePath)
    {
        std::ifstream in(filePath);
        if (!in)
            throw std::runtime_error("File open failed: " + filePath);
        json j;
        in >> j;
        return j;
    }
};

#endif // JSONFILEMANAGER_H