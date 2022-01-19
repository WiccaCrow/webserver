#pragma once

#include <map>
#include <string>

#include "HttpStatusCodes.hpp"
#include "Utils.hpp"
#include "HttpHeaders.hpp"

#define PARSED_NONE    0x0
#define PARSED_SL      0x1
#define PARSED_HEADERS 0x2
#define PARSED_BODY    0x3

namespace HTTP {

class Request {
    private:
    std::string                       _method;
    std::string                       _path;
    std::string                       _protocol;
    std::map<std::string, HeaderPair> _headers;
    std::string                       _body;
    uint8_t                           _parseFlags;

    public:
    Request();
    ~Request();

    const std::string                       &getMethod() const;
    const std::string                       &getPath() const;
    const std::string                       &getProtocol() const;
    const std::map<std::string, HeaderPair> &getHeaders() const;
    const std::string                       &getBody() const;
    const uint8_t                           &getFlags() const;

    void setFlag(uint8_t flag);
    void removeFlag(uint8_t flag);

    StatusCode parseStartLine(const std::string &line);
    StatusCode parseHeaders(const std::string &line);
    StatusCode parseBody(const std::string &line);

    private:
    StatusCode isValidMethod(const std::string &method);
    StatusCode isValidPath(const std::string &path);
    StatusCode isValidProtocol(const std::string &protocol);
};

}; // namespace HTTP