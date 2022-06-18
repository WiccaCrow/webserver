#pragma once

#include <stdint.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Time.hpp"

#ifndef LOGS_DIR
    # define LOGS_DIR "logs"
#endif

enum Levels {
    LOG_INFO   = 0b00000001,
    LOG_DEBUG  = 0b00000010,
    LOG_ERROR  = 0b00000100,
    LOG_SYSERR = 0b00001000
};

class Logger : private std::streambuf, public std::ostream {
    
private:
    std::string _logfile;
    bool        _logToFile;
    uint8_t     _flags;
    uint8_t     _flag;

    // For file logging
    std::ofstream _out;

    int overflow(int c);

public:
    Logger(void);
    ~Logger(void);

    void       setFlags(uint8_t flags);
    Logger     &info(void);
    Logger     &debug(void);
    Logger     &error(void);
    Logger     &syserr(void);
    Logger     &print(uint8_t);
    void        enableLogFile(void);

    template<typename T>
    Logger& operator<<(const T &val) {
        if (_flags & _flag) {
            if (_logToFile && _out.good()) {
                _out << val;
            }

            if ((_flag & LOG_ERROR) || (_flag & LOG_SYSERR)) {
                std::cerr << val;
            } else {
                std::cout << val;
            }
        }
        return *this;
    }

    Logger &operator<<(std::ostream& (*func)(std::ostream &));
};

extern Logger Log;
