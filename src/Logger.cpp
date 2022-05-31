#include "Logger.hpp"

Logger Log;

const int LOG_DEBUG = 1;
const int LOG_INFO  = 2;
const int LOG_ERROR = 4;

Logger::Logger()
    : _logfile("")
    , _logToFile(false)
    , _flags(0) {
}

void
Logger::setLogFile(const std::string &logfile) {
    _logToFile = true;
    _logfile   = logfile;

    _out.open(logfile.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!_out.good()) {
        std::cerr << "Logger: cannot open file " << logfile;
    }
}

Logger::~Logger() { }

void
Logger::setFlags(uint8_t flags) {
    _flags = flags;
}

void
Logger::print(uint8_t flag, const std::string &msg) {
    static const std::string _titles[5] = { "", "DEBUG", "INFO", "", "ERROR" };
    time_t timetoday;
    time(&timetoday);

    time_t cur = time(NULL);
    tm *ct = localtime(&cur);
    if (_flags & flag) {
        if (_logToFile && _out.good()) {
            _out << (ct->tm_mday) << "/" << (ct->tm_mon) + 1 << "/" << (ct->tm_year) + 1900 << " ";
            _out << (ct->tm_hour) << ":" << (ct->tm_min) << ":" << (ct->tm_sec);
            _out << " " << _titles[flag] << ": " << msg << std::endl;
        } else {
            std::cout << (ct->tm_mday) << "/" << (ct->tm_mon) + 1 << "/" << (ct->tm_year) + 1900 << " ";
            std::cout << (ct->tm_hour) << ":" << (ct->tm_min) << ":" << (ct->tm_sec);
            std::cout << " " << _titles[flag] << ": " << msg << std::endl;
        }
    }
}

void
Logger::debug(const std::string &s) {
    print(LOG_DEBUG, s);
}

void
Logger::error(const std::string &s) {
    print(LOG_ERROR, s);
}

void
Logger::info(const std::string &s) {
    print(LOG_INFO, s);
}
