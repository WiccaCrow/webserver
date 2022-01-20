#include "Request.hpp"

#include <iostream>

namespace HTTP {

Request::Request() : _parseFlags(PARSED_NONE) {}

Request::~Request() {}

const std::string &Request::getMethod() const {
    return _method;
}

const std::string &Request::getPath() const {
    return _path;
}

const std::string &Request::getProtocol() const {
    return _protocol;
}

const std::map<std::string, HeaderPair> &Request::getHeaders() const {
    return _headers;
}

const std::string &Request::getBody() const {
    return _body;
}

// Hide these methods
static void skipSpaces(const std::string &line, size_t &pos) {
    for (; line[pos] == ' '; pos++)
        ;
}

static std::string getData(const std::string &line, char delimiter, size_t &pos) {
    size_t tmp = pos;
    size_t end = pos = line.find(delimiter, pos);
    return line.substr(tmp, end - tmp);
}

StatusCode Request::parseStartLine(const std::string &line) {
    size_t pos = 0;

    _method = getData(line, ' ', pos);
    if (!isValidMethod(_method)) {
        return NOT_IMPLEMENTED;
    }

    skipSpaces(line, pos);
    _path = getData(line, ' ', pos);
    if (!isValidPath(_path)) {
        return BAD_REQUEST;
    }

    skipSpaces(line, pos);
    _protocol = getData(line, ' ', pos);

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

StatusCode Request::isValidMethod(const std::string &method) {
    std::string validMethods[9] = {
        "GET", "DELETE", "POST",
        "PUT", "HEAD", "CONNECT",
        "OPTIONS", "TRACE", "PATCH"};
    for (int i = 0; i < 9; ++i) {
        if (validMethods[i] == method)
            return CONTINUE;
    }
    return NOT_IMPLEMENTED;
}

StatusCode Request::isValidPath(const std::string &path) {
    if (path[0] != '/') {
        return BAD_REQUEST;
    }
    return CONTINUE;
}

StatusCode Request::isValidProtocol(const std::string &protocol) {
    char protocol_valid[] = "HTTP/1.1";
    if (protocol_valid == protocol)
        return CONTINUE;
    return HTTP_VERSION_NOT_SUPPORTED;
}

void Request::setFlag(uint8 flag) {
    _parseFlags |= flag;
}

void Request::removeFlag(uint8 flag) {
    _parseFlags &= ~flag;
}

const uint8 &Request::getFlags() const {
    return _parseFlags;
}

StatusCode Request::parseHeaders(const std::string &line) {
    if (line == "\r\n") {
        setFlag(PARSED_HEADERS);
        return CONTINUE;
    }

    size_t pos = 0;
    size_t delimiter = line.find(':', pos);

    std::string key = getData(line, ':', pos);
    std::string value = line.substr(delimiter + 1, line.length() - delimiter);

    trim(value, " \t\n\r");
    toLowerCase(key);
    toLowerCase(value);

    if (!validHeaders[crc(key.c_str(), key.length()) % tableSize]) {
        return BAD_REQUEST;
    }

    return CONTINUE;
}
}; // namespace HTTP