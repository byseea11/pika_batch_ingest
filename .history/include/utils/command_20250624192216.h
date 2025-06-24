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
        iError,
    };

    Result() = default;
    Result(const std::string &msg, Ret ret = iNone)
        : msg_(msg), ret_(ret) {}

    const std::string &getMessage() const { return msg_; }
    Ret getRet() const { return ret_; }
    void Ok(const std::string &msg) const
    {
        this.ret_ = iOk;
        this->msg_ = msg;
    }
    void Error() const { return ret_ == iError; }

private:
    std::string msg_;
    Ret ret_ = iNone;
};

#endif