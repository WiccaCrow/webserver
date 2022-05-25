#pragma once

#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include "CRC.hpp"
#include "Header.hpp"
#include "Logger.hpp"
#include "StatusCodes.hpp"
#include "Types.hpp"
#include "Utils.hpp"
#include "ValidateHeaders.hpp"
#include "ServerBlock.hpp"

#define PARSED_NONE    0x0
#define PARSED_SL      0x1
#define PARSED_HEADERS 0x2
#define PARSED_BODY    0x4

namespace HTTP {

class Request {
    private:
    std::string              _method;
    std::string              _path;
    std::string              _protocol;
    std::string              _queryString;
    std::string              _scriptName;
    std::map<uint32, Header> _headers;
    HTTP::StatusCode         _status;

    ServerBlock              &_servBlock;

    bool        _isSizeChunk;
    long        _sizeChunk;
    std::string _body;
    uint8       _parseFlags;

    public:
    Request(ServerBlock &servBlock);
    ~Request();

    Request(const Request &other);
    Request &operator=(const Request &other);

    const ServerBlock              &getServerBlock() const;

    const std::string              &getMethod() const;
    const std::string              &getPath() const;
    const std::string              &getProtocol() const;
    const std::map<uint32, Header> &getHeaders() const;
    const std::string              &getBody() const;
    const uint8                    &getFlags() const;
    const HTTP::StatusCode         &getStatus() const;

    // Needed to be improved
    const std::string              &getQueryString() const;
    const std::string              &getScriptName() const;
    const char *  getHeaderValue(HeaderCode key) const;

    bool empty();

    void setFlag(uint8 flag);
    void removeFlag(uint8 flag);
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
    bool getChunked_isSizeChunk();
    void setChunked_isSizeChunk(bool isSize);
    long getChunked_Size();
    void setChunked_Size(long size);
    void setStatus(const HTTP::StatusCode &status);

};

}; // namespace HTTP