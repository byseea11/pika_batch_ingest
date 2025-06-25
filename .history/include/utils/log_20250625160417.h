#ifndef LOG_H
#define LOG_H
#include <string>
#include <iostream>
#include <fstream>

class Log
{
public:
    enum LogLevel
    {
        Info = 0,
        Warning,
        Error
    };

    Log() = default;

    // 设置日志级别和内容
    void setLog(LogLevel level, const std::string &msg)
    {
        level_ = level;
        message_ = msg;
    }

    // 输出日志（根据日志级别不同输出格式不同）
    void logMessage() const
    {
        switch (level_)
        {
        case Info:
            std::cout << "[INFO] " << message_ << std::endl;
            break;
        case Warning:
            std::cout << "[WARNING] " << message_ << std::endl;
            break;
        case Error:
            std::cerr << "[ERROR] " << message_ << std::endl;
            break;
        }
    }

    // 可以添加将日志保存到文件的功能
    void logToFile(const std::string &fileName) const
    {
        std::ofstream logFile(fileName, std::ios::app); // 以追加模式打开文件
        if (logFile.is_open())
        {
            switch (level_)
            {
            case Info:
                logFile << "[INFO] " << message_ << std::endl;
                break;
            case Warning:
                logFile << "[WARNING] " << message_ << std::endl;
                break;
            case Error:
                logFile << "[ERROR] " << message_ << std::endl;
                break;
            }
            logFile.close();
        }
    }

private:
    LogLevel level_ = Info;
    std::string message_;
};

#define LOG_INFO(content)                     \
    log.setLog(Log::LogLevel::Info, content); \
    log.logMessage();                         \
    log.logToFile("log.txt"); // 输出到控制台并保存到文件
#define LOG_WARN(content) log.setLog(Log::LogLevel::Warning, content);
#define LOG_ERROR(content) log.setLog(Log::LogLevel::Error, content);

#endif
