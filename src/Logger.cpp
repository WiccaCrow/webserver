#include "Logger.hpp"

Logger Log;

Logger::Logger() : std::ostream(this)
    , _logfile("")
    , _logToFile(false)
    , _flags(0) {
}

Logger::~Logger() { }

int
Logger::overflow(int c) {
    std::cout.put(c);
    _out.put(c);
    return 0;
}

Logger &
Logger::operator<<(std::ostream& (*func)(std::ostream &)) {
    if (_flags & _flag) {
        if (_logToFile && _out.good()) {
            func(_out);
        }

        if ((_flag & LOG_ERROR) || (_flag & LOG_SYSERR)) {
            func(std::cerr);
        } else {
            func(std::cout);
        }
    }
    return *this;
}

void
Logger::enableLogFile(void) {

    const std::string logDir = LOGS_DIR"/";
    const std::string logFile = makeTimeString("%d-%m-%Y_%H-%M-%S") + ".log";

    _logToFile = true;
    _logfile   = logDir + logFile;

    _out.open(_logfile.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!_out.good()) {
        std::cerr << "Logger: cannot open/create file " << _logfile << std::endl;
    } else {
        std::cout << "Logger: logging into " << _logfile.c_str() << std::endl;
    }
}


void
Logger::setFlags(uint8_t flags) {
    _flags = flags;
}

std::string 
Logger::makeTimeString(const char *format) {
    time_t cur = time(NULL);
    tm *ct = localtime(&cur);

    char buff[25];
    strftime(buff, sizeof(buff), format, ct);
   
    return buff;
}

Logger &
Logger::print(uint8_t lvl) {
    _flag = lvl;
    static const char *_titles[9] = { "", "  INFO", " DEBUG", "", " ERROR", "", "", "", "SYSERR" };

    std::string statusLine = makeTimeString() + " " + _titles[_flag] + ": ";

    return this->operator<<(statusLine);
}

Logger &
Logger::debug(void) {
    return print(LOG_DEBUG);
}

Logger &
Logger::error(void) {
    return print(LOG_ERROR);
}

Logger &
Logger::syserr(void) {
    print(LOG_SYSERR) << "errno [" << errno << "]: " << strerror(errno) << std::endl;
    return print(LOG_SYSERR);
}

Logger &
Logger::info(void) {
    return print(LOG_INFO);
}
