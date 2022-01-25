#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "Types.hpp"

#define DEBUG 1
#define INFO  2
#define ERROR 4

const std::string _titles[5] = {"", "DEBUG", "INFO", "", "ERROR"};

class Logger {
    private:
    std::string _logfile;
    bool        _logToFile;
    uint8       _flags;

    std::ofstream _out;

    public:
    Logger();
    ~Logger();

    void setLogFile(const std::string &logfile);
    void setFlags(uint8 flags);
    void print(uint8 flag, const std::string &msg);
    void debug(const std::string &s);
    void error(const std::string &s);
    void info(const std::string &s);
};

extern Logger Log;