#include "Request.hpp"

Request::Request() : _parseFlags(PARSED_NONE) {}

Request::~Request() {}

// Hide these methods
static void skipSpaces(const std::string &line, size_t &pos) {
    for (; line[pos] == ' '; pos++)
        ;
}

static std::string getData(const std::string &line, size_t &pos) {
    size_t tmp = pos;
    size_t end = pos = line.find(' ', pos);
    return line.substr(tmp, end - tmp);
}

using namespace HTTP;

int Request::parseStartLine(const std::string &line) {
    size_t pos = 0;

    _method = getData(line, pos);
    if (!isValidMethod(_method)) {
        return NOT_IMPLEMENTED;
    }

    skipSpaces(line, pos);
    _path = getData(line, pos);
    if (!isValidPath(_path)) {
        return BAD_REQUEST;
    }

    skipSpaces(line, pos);
    _protocol = getData(line, pos);

    skipSpaces(line, pos);
    if (line[pos]) {
        return BAD_REQUEST;
    }
    if (!isValidProtocol(_protocol)) {
        // Or 505, need to improve
        return BAD_REQUEST;
    }

    setFlag(PARSED_SL);
    return CONTINUE;
}

bool Request::isValidMethod(const std::string &method) {
    std::string validMethods[9] = {
        "GET", "DELETE", "POST",
        "PUT", "HEAD", "CONNECT",
        "OPTIONS", "TRACE", "PATCH"};

    for (int i = 0; i < 9; ++i) {
        if (validMethods[i] == method)
            return true;
    }
    return false;
}

bool Request::isValidPath(const std::string &path) {
    return path[0] == '/';
}

bool Request::isValidProtocol(const std::string &protocol) {
    return protocol == "HTTP/1.1";
}

void Request::setFlag(unsigned char flag) {
    _parseFlags |= flag;
}

void Request::removeFlag(unsigned char flag) {
    _parseFlags &= ~flag;
}

unsigned char Request::getFlags() const {
    return _parseFlags;
}