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
        std::ofstream logFile(fileName, std::ios::app);
        if (logFile.is_open())
        {
            auto now = std::chrono::system_clock::now();
            auto now_time = std::chrono::system_clock::to_time_t(now);
            logFile << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") << " ";

            switch (level_)
            {
            case Info:
                logFile << "[INFO] ";
                break;
            case Warning:
                logFile << "[WARNING] ";
                break;
            case Error:
                logFile << "[ERROR] ";
                break;
            }
            logFile << message_ << std::endl;
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
#define LOG_WARN(content)                        \
    log.setLog(Log::LogLevel::Warning, content); \
    log.logMessage();                            \
    log.logToFile("log.txt"); // 输出到控制台并保存到文件
#define LOG_ERROR(content)                     \
    log.setLog(Log::LogLevel::Error, content); \
    log.logMessage();                          \
    log.logToFile("log.txt"); // 输出到控制台并保存到文件

#endif
