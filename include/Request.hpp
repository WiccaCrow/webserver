#pragma once

#include <map>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "CRC.hpp"
#include "Utils.hpp"
#include "Logger.hpp"
#include "Header.hpp"
#include "Globals.hpp"
#include "Location.hpp"
#include "ServerBlock.hpp"
#include "StatusCodes.hpp"
#include "ValidHeaders.hpp"

#define PARSED_NONE    0x0
#define PARSED_SL      0x1
#define PARSED_HEADERS 0x2
#define PARSED_BODY    0x4

namespace HTTP {

class Request {

private:
    std::string                  _method;
    std::string                  _uri;
    std::string                  _protocol;
    std::string                  _path;
    std::string                  _queryString;
    std::string                  _scriptName;
    std::map<HeaderCode, Header> _headers;
    HTTP::StatusCode             _status;

    ServerBlock                 *_servBlock;
    Location                    *_location;

    bool          _flag_getline_bodySize;
    unsigned long _bodySize;
    std::string   _body;
    uint8_t       _parseFlags;

public:
    Request(ServerBlock *servBlock);
    ~Request();

    Request(const Request &other);
    Request &operator=(const Request &other);

    const ServerBlock *getServerBlock() const;

    const std::string                  &getPath() const;
    const std::string                  &getMethod() const;
    const std::string                  &getURI() const;
    const std::string                  &getProtocol() const;
    const std::map<HeaderCode, Header> &getHeaders() const;
    const std::string                  &getBody() const;
    const uint8_t                      &getFlags() const;
    const HTTP::StatusCode             &getStatus() const;
    Location                           *getLocationPtr();

    // Needed to be improved
    const std::string &getQueryString() const;
    const std::string &getScriptName() const;
    const char        *getHeaderValue(HeaderCode key) const;

    bool empty();

    void setFlag(uint8_t flag);
    void removeFlag(uint8_t flag);
    void clear(void);

    StatusCode parseStartLine(const std::string &line);
    StatusCode parseHeader(std::string line);
    StatusCode parseChunked(const std::string &line);
    StatusCode parseBody(const std::string &line);
    StatusCode parseLine(std::string line);

private:
    StatusCode isValidMethod(const std::string &method);
    StatusCode isValidPath(const std::string &path);
    StatusCode isValidProtocol(const std::string &protocol);

public:
    // for chunked
    bool          getBodySizeFlag();
    void          setBodySizeFlag(bool isSize);
    unsigned long getBodySize();
    void          setBodySize(unsigned long size);
    void          setStatus(const HTTP::StatusCode &status);
};

} // namespace HTTP
