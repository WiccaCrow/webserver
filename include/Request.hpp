#pragma once

#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include "CRC.h"
#include "Header.hpp"
#include "Logger.hpp"
#include "StatusCodes.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "ValidateHeaders.hpp"

#define PARSED_NONE    0x0
#define PARSED_SL      0x1
#define PARSED_HEADERS 0x2
#define PARSED_BODY    0x3

namespace HTTP {

class Request {
    private:
    std::string              _method;
    std::string              _path;
    std::string              _protocol;
    std::map<uint32, Header> _headers;
    HTTP::StatusCode         _status;

    std::string _body;
    uint8       _parseFlags;

    public:
    Request();
    ~Request();

    std::string &             getMethod();
    std::string &             getPath();
    std::string &             getProtocol();
    std::map<uint32, Header> &getHeaders();
    std::string &             getBody();
    uint8 &                   getFlags();

    void setFlag(uint8 flag);
    void removeFlag(uint8 flag);

    StatusCode parseStartLine(const std::string &line);
    StatusCode parseHeader(std::string line);
    StatusCode parseBody(const std::string &line);
    StatusCode parseLine(std::string line);

    private:
    StatusCode isValidMethod(const std::string &method);
    StatusCode isValidPath(const std::string &path);
    StatusCode isValidProtocol(const std::string &protocol);
};

}; // namespace HTTP