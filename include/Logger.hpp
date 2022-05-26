#pragma once

#include <fstream>
#include <iostream>
#include <string>

#define DEBUG 1
#define INFO  2
#define ERROR 4

const std::string _titles[5] = {"", "DEBUG", "INFO", "", "ERROR"};

class Logger {
    private:
    std::string _logfile;
    bool        _logToFile;
    uint8_t       _flags;

    std::ofstream _out;

    public:
    Logger();
    ~Logger();

    void setLogFile(const std::string &logfile);
    void setFlags(uint8_t flags);
    void print(uint8_t flag, const std::string &msg);
    void debug(const std::string &s);
    void error(const std::string &s);
    void info(const std::string &s);
};

extern Logger Log;