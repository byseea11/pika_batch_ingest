#include <mutex> // 新增头文件

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

    void setLog(LogLevel level, const std::string &msg)
    {
        std::lock_guard<std::mutex> lock(mutex_); // 加锁保护日志内容
        level_ = level;
        message_ = msg;
    }

    void logMessage() const
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

    void logToFile(const std::string &fileName) const
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
    LogLevel level_ = Info;
    std::string message_;
};

#define LOG(level, content)                    \
    log.setLog(Log::LogLevel::level, content); \
    log.logMessage();                          \
    log.logToFile("log.txt")

#define LOG_INFO(content) LOG(Info, content)
#define LOG_WARN(content) LOG(Warning, content)
#define LOG_ERROR(content) LOG(Error, content)
