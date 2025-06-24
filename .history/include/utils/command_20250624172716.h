#ifndef COMMAND_H
#define COMMAND_H
#include <string>

const std::string kNewLine = "\r\n";

class Result
{
public:
    enum Ret
    {
        kNone = 0,
        kOk,
    };

private:
    std::string msg_;
    Ret ret_ = kNone;
};

#endif