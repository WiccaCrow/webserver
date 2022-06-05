#pragma once

#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>

#include "CRC.hpp"
#include "Globals.hpp"
#include "Header.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "ServerBlock.hpp"
#include "Status.hpp"
#include "URI.hpp"
#include "Utils.hpp"

#define PARSED_NONE    0x0
#define PARSED_SL      0x1
#define PARSED_HEADERS 0x2
#define PARSED_BODY    0x4

namespace HTTP {

class Client;

class Request {

private:
    std::string                  _method;
    URI                          _uri;
    std::string                  _rawURI;
    std::string                  _protocol;
    std::string                  _resolvedPath;
    std::map<HeaderCode, Header> _headers;
    HTTP::StatusCode             _status;
    int                          _major : 4;
    int                          _minor : 4;

    ServerBlock *_servBlock;
    Location    *_location;
    Client      *_client;

    bool          _flag_getline_bodySize;
    unsigned long _bodySize;
    std::string   _body;
    uint8_t       _parseFlags;

public:
    Request();
    ~Request();

    Request(const Request &other);
    Request &operator=(const Request &other);

    const ServerBlock *getServerBlock() const;
    void               setServerBlock(ServerBlock *);

    const Client *getClient() const;
    void          setClient(Client *);

    const std::string                  &getPath() const;
    const std::string                  &getMethod() const;
    const std::string                  &getProtocol() const;
    const std::map<HeaderCode, Header> &getHeaders() const;
    const std::string                  &getBody() const;
    const uint8_t                      &getFlags() const;
    const HTTP::StatusCode             &getStatus() const;
    Location                           *getLocationPtr();
    URI                                &getUriRef();
    const std::string                  &getRawUri() const;

    // Needed to be improved
    const std::string &getQueryString() const;
    const std::string &getResolvedPath() const;
    const std::string  getHeaderValue(HeaderCode key) const;

    bool empty();

    void setFlag(uint8_t flag);
    void removeFlag(uint8_t flag);
    void clear(void);

    StatusCode parseStartLine(const std::string &line);
    StatusCode parseHeader(std::string line);
    StatusCode parseChunked(const std::string &line);
    StatusCode writeChunkedSize(const std::string &line);
    StatusCode parseBody(const std::string &line);
    StatusCode writeBody(const std::string &line);
    StatusCode parseLine(std::string line);

private:
    bool isValidMethod(const std::string &method);
    // bool isValidPath(const std::string &path);
    bool isValidProtocol(const std::string &protocol);

public:
    // for chunked
    bool          getBodySizeFlag();
    void          setBodySizeFlag(bool isSize);
    unsigned long getBodySize();
    void          setBodySize(unsigned long size);
    void          setStatus(const HTTP::StatusCode &status);
};

} // namespace HTTP
