#include "Logger.hpp"
#include <vector>

Logger Log;

static const std::vector<std::string> initTitles(void) {
    std::vector<std::string> titles(9);
    
    titles[LOG_INFO] = "INFO";
    titles[LOG_DEBUG] = "DEBUG";
    titles[LOG_ERROR] = "ERROR";
    titles[LOG_SYSERR] = "SYSERR";

    size_t max = 0;
    for (size_t i = 0; i < titles.size(); i++) {
        max = titles[i].length() > max ? titles[i].length() : max;
    }

    for (size_t i = 0; i < titles.size(); i++) {
        if (!titles[i].empty()) {
            titles[i] += std::string(max - titles[i].length(), ' ');
        }
    }
    return titles;
}

static const std::vector<std::string> titles = initTitles();

Logger::Logger() : std::ostream(this)
    , _logfile("")
    , _logToFile(false)
    , _flags(0) {}

Logger::~Logger() { }

void
Logger::enableLogFile(void) {

    const std::string logDir = LOGS_DIR"/";
    const std::string logFile = Time::local("%d-%m-%Y_%H-%M-%S") + ".log";

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

Logger &
Logger::print(uint8_t flag) {
    _flag = flag;

    return *this << Time::local() << " " << titles[_flag] << " ";
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