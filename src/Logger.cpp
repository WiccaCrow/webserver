#include "Logger.hpp"

Logger Log;

Logger::Logger() : _logfile(""), _logToFile(false), _flags(0) {
}

void Logger::setLogFile(const std::string &logfile) {
    _logToFile = true;
    _logfile = logfile;

    _out.open(logfile.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!_out.good()) {
        std::cerr << "Logger: cannot open file " << logfile << std::endl;
    }
}

Logger::~Logger() {}

void Logger::setFlags(uint8 flags) {
    _flags = flags;
}

void Logger::print(uint8 flag, const std::string &msg) {
    if (_flags & flag) {
        if (_logToFile && _out.good()) {
            _out << _titles[flag] << ": " << msg << std::endl;
        } else {
            std::cout << _titles[flag] << ": " << msg << std::endl;
        }
    }
}

void Logger::debug(const std::string &s) {
    print(DEBUG, s);
}

void Logger::error(const std::string &s) {
    print(ERROR, s);
}

void Logger::info(const std::string &s) {
    print(INFO, s);
}