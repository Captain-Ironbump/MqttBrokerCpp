#ifndef LOGGER_H
#define LOGGER_H

// #include <ctime>
#include <fstream>

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
private:
    // Variables
    size_t maxFileSize = 1024 * 1024 * 10; // 10 MB
    std::ofstream* logFile = nullptr;
    int fileCount;
    bool useLogFile = false;
    // Functions
    std::string levelToString(LogLevel level);
    std::string getColorForLevel(LogLevel level);
public:
    Logger(const std::string& baseFileName);
    ~Logger();
    void log(LogLevel level, const char* message, ...);
    std::string getNextLogFile(const std::string& baseFileName);
};

#endif
