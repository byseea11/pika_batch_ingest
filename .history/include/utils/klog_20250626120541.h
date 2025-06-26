#ifndef KLOG_H
#define KLOG_H
#include <mutex>
#include <iostream>

class KLog
{
public:
    enum KLogLevel
    {
        Info = 0,
        Warning,
        Error
    };

    KLog() = default;

    void setKLog(KLogLevel level, const std::string &msg)
    {
        std::lock_guard<std::mutex> lock(mutex_); // 加锁保护日志内容
        level_ = level;
        message_ = msg;
    }

    void KLogMessage() const
    {
        std::lock_guard<std::mutex> lock(mutex_); // 保证控制台输出线程安全
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

    void LogToFile(const std::string &fileName) const
    {
        std::lock_guard<std::mutex> lock(mutex_); // 保证文件输出线程安全
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
    mutable std::mutex mutex_; // 互斥锁保护并发访问
    KLogLevel level_ = Info;
    std::string message_;
};

#define KLog(level, content)                       \
    KLog.setKLog(KLog::KLogLevel::level, content); \
    KLog.KLogMessage();                            \
    KLog.KLogToFile("KLog.txt")

#define Log_INFO(content) KLog(Info, content)
#define Log_WARN(content) KLog(Warning, content)
#define Log_ERROR(content) KLog(Error, content)

#endif // KLOG_H