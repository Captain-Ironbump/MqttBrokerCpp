#ifndef LOGGER_H
#define LOGGER_H

// #include <ctime>
#include <fstream>
#include <sstream>

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
    std::ofstream* logFile = nullptr;
    // Functions
    std::string levelToString(LogLevel level);
    std::string getColorForLevel(LogLevel level);
public:
    Logger(const std::string& filename);
    ~Logger();
    void log(LogLevel level, const std::string& message, int useLogFile=0);
};

#endif
