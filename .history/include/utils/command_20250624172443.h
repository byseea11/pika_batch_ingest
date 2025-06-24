#ifndef COMMAND_H
#define COMMAND_H
#include <string>

class Result
{
public:
    enum Ret
    {
        kNone = 0,
        kOk,
    };

private:
    std::string msg;
    Ret ret;
};

#endif