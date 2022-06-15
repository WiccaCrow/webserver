#include "Logger.hpp"

Logger Log;

const int LOG_DEBUG = 1;
const int LOG_INFO  = 2;
const int LOG_ERROR = 4;
const int LOG_SYSERR = 8;

Logger::Logger()
    : _logfile("")
    , _logToFile(false)
    , _flags(0) {
}

void
Logger::enableLogFile(void) {

    _logToFile = true;
    _logfile   = std::string(LOGS_DIR) + "/" + Log.makeTimeString('-','_','-') + ".log";

    _out.open(_logfile.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!_out.good()) {
        std::cerr << "Logger: cannot open/create file " << _logfile << std::endl;
    } else {
        std::cout << "Logger: log into " << _logfile.c_str() << std::endl;
    }
}

Logger::~Logger() { }

void
Logger::setFlags(uint8_t flags) {
    _flags = flags;
}

static void
addNumber(std::stringstream &ss, int num) {
    if (num < 10) {
        ss << std::string("0");
    }
    ss << num;
}

std::string 
Logger::makeTimeString(char dateSep, char sep, char timeSep) {
    time_t cur = time(NULL);
    tm *ct = localtime(&cur);

    std::stringstream ss;
    addNumber(ss, ct->tm_mday);
    ss << dateSep;
    addNumber(ss, ct->tm_mon + 1);
    ss << dateSep;
    addNumber(ss, ct->tm_year + 1900);
    ss << sep;
    addNumber(ss, ct->tm_hour);
    ss << timeSep;
    addNumber(ss, ct->tm_min);
    ss << timeSep;
    addNumber(ss, ct->tm_sec);
    return ss.str();
}

void
Logger::print(uint8_t flag, const std::string &msg) {
    static const char *_titles[9] = { "", "DEBUG", "INFO", "", "ERROR", "", "", "", "SYSERR" };

    if (_flags & flag) {
        if (_logToFile && _out.good()) {
            _out << makeTimeString() << " " << _titles[flag] << ": " << msg << std::endl;
        }

        if (_flags & LOG_ERROR) {
            std::cerr << makeTimeString() << " " << _titles[flag] << ": " << msg << std::endl;
        } else {
            std::cout << makeTimeString() << " " << _titles[flag] << ": " << msg << std::endl;
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
Logger::syserr(const std::string &s) {
    std::ostringstream errnoinfo;
    errnoinfo << "errno: " << errno << ": " << strerror(errno);
    print(LOG_SYSERR, s);
    print(LOG_SYSERR, errnoinfo.str());
}

void
Logger::info(const std::string &s) {
    print(LOG_INFO, s);
}
