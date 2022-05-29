#include "Logger.hpp"

Logger Log;

const int LOG_DEBUG = 1;
const int LOG_INFO = 2;
const int LOG_ERROR = 4;

Logger::Logger() : _logfile(""), _logToFile(false), _flags(0) {
}

void Logger::setLogFile(const std::string &logfile) {
    _logToFile = true;
    _logfile = logfile;

    _out.open(logfile.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!_out.good()) {
        std::cerr << "Logger: cannot open file " << logfile;
    }
}

Logger::~Logger() {}

void Logger::setFlags(uint8_t flags) {
    _flags = flags;
}

void Logger::print(uint8_t flag, const std::string &msg) {
    static const std::string _titles[5] = { "", "DEBUG", "INFO", "", "ERROR" };
    if (_flags & flag) {
        if (_logToFile && _out.good()) {
            _out << _titles[flag] << ": " << msg << std::endl;
        } else {
            std::cout << _titles[flag] << ": " << msg << std::endl;
        }
    }
}

void Logger::debug(const std::string &s) {
    print(LOG_DEBUG, s);
}

void Logger::error(const std::string &s) {
    print(LOG_ERROR, s);
}

void Logger::info(const std::string &s) {
    print(LOG_INFO, s);
}
