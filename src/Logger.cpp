#include "Logger.hpp"
#include <vector>

Logger Log;

static const std::vector<std::string> initTitles(void) {
    std::vector<std::string> titles(9);
    
    titles[LOG_SYSERR] = "SYSERR";
    titles[LOG_ERROR] = "ERROR";
    titles[LOG_INFO] = "INFO";
    titles[LOG_DEBUG] = "DEBUG";

    std::size_t max = 0;
    for (std::size_t i = 0; i < titles.size(); i++) {
        max = titles[i].length() > max ? titles[i].length() : max;
    }

    for (std::size_t i = 0; i < titles.size(); i++) {
        if (!titles[i].empty()) {
            titles[i] += std::string(max - titles[i].length(), ' ');
        }
    }

    return titles;
}

static const std::vector<std::string> titles = initTitles();

pthread_mutex_t Logger::_lock_print;

Logger::Logger() : std::ostream(this)
    , _logfile("")
    , _logDir(LOGS_DIR"/")
    , _logToFile(false)
    , _logToStd(true)
    , _curLevel(0)
    , _askLevel(0) {
    pthread_mutex_init(&_lock_print, NULL);
}

Logger::~Logger() {
    pthread_mutex_destroy(&_lock_print);
}

void
Logger::setLogDir(const std::string &dir) {
    _logDir = dir;
    if (!endsWith(_logDir, "/")) {
        _logDir += '/';
    }
}

void
Logger::logToStd(bool flag) {
    _logToStd = flag;
}

void
Logger::logToFile(bool flag) {

    if (_logToFile == flag) {
        return ;
    }

    _logToFile = flag;
    if (_logToFile) {
        _logfile = _logDir + Time::local("%d-%m-%Y_%H-%M-%S") + ".log";

        _out.open(_logfile.c_str(), std::ios_base::out | std::ios_base::trunc);

        if (!_out.good()) {
            this->error() << "Cannot open/create logfile " << _logfile << Log.endl;
        } else {
            this->info() << "Logging into " << _logfile.c_str() << Log.endl;
        }
    } else {
        _out.close();
        _out.clear();
    }
}

void
Logger::setLevel(uint8_t level) {
    _curLevel = level;
}

Logger &
Logger::print(uint8_t level) {
    pthread_mutex_lock(&_lock_print);
    _askLevel = level;

    return *this << Time::local() << " " << titles[_askLevel] << " ";
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
    print(LOG_SYSERR) << "errno [" << errno << "]: " << strerror(errno) << Log.endl;
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
    if (_askLevel <= _curLevel) {
        if (_logToFile && _out.good()) {
            func(_out);
        }

        if ((_askLevel == LOG_ERROR) || (_askLevel == LOG_SYSERR)) {
            func(std::cerr);
        } else {
            func(std::cout);
        }
    }
    pthread_mutex_unlock(&_lock_print);
    return *this;
}

std::ostream&
Logger::cr(std::ostream& out) {
    out << '\r';
    out.flush();
    return out;
}

std::ostream&
Logger::flush(std::ostream& out) {
    out << std::flush;
    return out;
}

std::ostream&
Logger::endl(std::ostream& out) {
    out << std::endl;
    return out;
}

std::ostream&
Logger::ends(std::ostream& out) {
    out << std::ends;
    return out;
}