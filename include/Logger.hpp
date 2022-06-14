#pragma once

#include <stdint.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifndef LOGS_DIR
#define LOGS_DIR "logs"
#endif
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

    void        setFlags(uint8_t flags);
    void        info(const std::string &s);
    void        debug(const std::string &s);
    void        error(const std::string &s);
    void        syserr(const std::string &s);
    void        enableLogFile(void);
    void        print(uint8_t flag, const std::string &msg);
    std::string makeTimeString(char dateSep = '/', char sep = ' ', char timeSep = ':');
};

extern const int LOG_DEBUG;
extern const int LOG_INFO;
extern const int LOG_ERROR;
extern const int LOG_SYSERR;

extern Logger Log;
