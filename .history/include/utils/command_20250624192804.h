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
        iWarning,
        iInfo,
        iError,
    };

    Result() = default;

    Result(const std::string &msg, Ret ret = iNone)
        : msg_(msg), ret_(ret) {}

    const std::string &getMessage() const { return msg_; }
    Ret getRet() const { return ret_; }

    // 允许链式调用，返回当前对象
    Result &Ok(const std::string &msg)
    {
        ret_ = Ret::iOk;
        msg_ = msg;
        return *this;
    }

    Result &Error(const std::string &msg)
    {
        ret_ = Ret::iError;
        msg_ = msg;
        return *this;
    }

    Result &Warn(const std::string &msg)
    {
        ret_ = Ret::iWarning;
        msg_ = msg;
        return *this;
    }

    Result &Info(const std::string &msg)
    {
        ret_ = Ret::iInfo;
        msg_ = msg;
        return *this;
    }

    // 静态工厂方法，让 Result 类的创建更加直观
    static Result OkResult(const std::string &msg)
    {
        return Result(msg, iOk);
    }

    static Result ErrorResult(const std::string &msg)
    {
        return Result(msg, iError);
    }

private:
    std::string msg_;
    Ret ret_ = iNone;
};

#endif
