#pragma once

#include <stdint.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <pthread.h>

#include "Time.hpp"
#include "Utils.hpp"

#ifndef LOGS_DIR
    # define LOGS_DIR "logs"
#endif

enum Levels {
    LOG_SYSERR = 0b00000001,
    LOG_ERROR  = 0b00000010,
    LOG_INFO   = 0b00000100,
    LOG_DEBUG  = 0b00001000,
};

class Logger : private std::streambuf, public std::ostream {
    
private:
    std::string _logfile;
    std::string _logDir;
    bool        _logToFile;
    bool        _logToStd;
    uint8_t     _curLevel;
    uint8_t     _askLevel;
    static pthread_mutex_t _lock_print;

    // For file logging
    std::ofstream _out;

    int overflow(int c);

public:
    Logger(void);
    ~Logger(void);

    void       setLevel(uint8_t level);
    Logger     &info(void);
    Logger     &debug(void);
    Logger     &error(void);
    Logger     &syserr(void);
    Logger     &print(uint8_t);
    void        logToFile(bool);
    void        logToStd(bool);
    void        setLogDir(const std::string &);

    Logger &operator<<(std::ostream& (*func)(std::ostream &));
    
    template<typename T>
    Logger& operator<<(const T &val) {
        if (_askLevel <= _curLevel) {
            if (_logToFile && _out.good()) {
                _out << val;
            }

            if (_logToStd) {
                if ((_askLevel == LOG_ERROR) || (_askLevel == LOG_SYSERR)) {
                    std::cerr << val;
                } else {
                    std::cout << val;
                }
            }
        }
        return *this;
    }

    static std::ostream& cr(std::ostream& out);
    static std::ostream& flush(std::ostream& out);
    static std::ostream& endl(std::ostream& out);
    static std::ostream& ends(std::ostream& out);

};

extern Logger Log;
