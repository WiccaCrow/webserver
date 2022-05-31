#pragma once

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <string>
#include <sstream>


class Logger {

private:
    std::string _logfile;
    bool        _logToFile;
    uint8_t     _flags;

    // For file logging
    std::ofstream _out;

public:
    Logger();
    ~Logger();

    void setFlags(uint8_t flags);
    void info(const std::string &s);
    void debug(const std::string &s);
    void error(const std::string &s);
    void setLogFile(const std::string &logfile);
    void print(uint8_t flag, const std::string &msg);
};

extern const int LOG_DEBUG;
extern const int LOG_INFO;
extern const int LOG_ERROR;

extern Logger Log;
