#pragma once

#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include "HttpStatusCodes.hpp"

#define PARSED_NONE    0x0
#define PARSED_SL      0x1
#define PARSED_HEADERS 0x2
#define PARSED_BODY    0x3

class Request {
    private:
    std::string                        _method;
    std::string                        _path;
    std::string                        _protocol;
    std::map<std::string, std::string> _headers;
    std::string                        _body;
    unsigned char                      _parseFlags;

    public:
    Request();
    ~Request();

    int           parseStartLine(const std::string &line);
    void          setFlag(unsigned char flag);
    void          removeFlag(unsigned char flag);
    unsigned char getFlags() const;

    private:
    bool                                 isValidMethod(const std::string &method);
    bool                                 isValidPath(const std::string &path);
    bool                                 isValidProtocol(const std::string &protocol);
    std::pair<std::string, std::string> &parseHeader(const std::string &line);
};