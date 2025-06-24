#ifndef COMMAND_H
#define COMMAND_H
#include <string>

const std::string iNewLine = "\r\n";

// 返回结果类，表示操作的最终结果（成功、失败等）
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

    void Ok(const std::string &msg)
    {
        ret_ = Ret::iOk;
        msg_ = msg;
    }

    void Error(const std::string &msg)
    {
        ret_ = Ret::iError;
        msg_ = msg;
    }

    void None(const std::string &msg)
    {
        ret_ = Ret::iNone;
        msg_ = msg;
    }

private:
    std::string msg_;
    Ret ret_ = iNone;
};

#endif
