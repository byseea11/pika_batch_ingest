#ifndef COMMAND_H
#define COMMAND_H
#include <string>

const std::string iNewLine = "\r\n";

class Result
{
public:
    enum Ret
    {
        iNone = 0,
        iOk,
    };

private:
    std::string msg_;
    Ret ret_ = iNone;
};

#endif