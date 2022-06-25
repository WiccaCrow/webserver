#pragma once

#include <climits>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>

#include "CRC.hpp"
#include "Globals.hpp"
#include "Location.hpp"
#include "Logger.hpp"
#include "RequestHeader.hpp"
#include "ServerBlock.hpp"
#include "Status.hpp"
#include "URI.hpp"
#include "Utils.hpp"
#include "Time.hpp"
#include "Range.hpp"

#define PARSED_NONE    0x0
#define PARSED_SL      0x1
#define PARSED_HEADERS 0x2
#define PARSED_BODY    0x4

namespace HTTP {

class Client;

class Request {

private:
    std::string                       _method;
    URI                               _uri;
    std::string                       _rawURI;
    std::string                       _protocol;
    std::string                       _resolvedPath;
    URI                               _host;
    URI                               _referrer;
    std::map<uint32_t, RequestHeader> _headers;
    StatusCode                        _status;
    int                               _major : 4;
    int                               _minor : 4;

    ServerBlock *_servBlock;
    Location    *_location;
    Client      *_client;

    bool          _isChuckSize;
    size_t        _chunkSize;
    size_t        _bodySize;
    std::string   _body;
    uint8_t       _parseFlags;
    RangeList     _ranges;

    bool     _isAuthorized;
    uint32_t _storedHash;

    bool _isFormed;
    bool _chuckedRequested;

    std::map<std::string, std::string> _cookie;

public:
    Request();
    Request(Client *);
    ~Request();

    Request(const Request &other);
    Request &operator=(const Request &other);

    ServerBlock *getServerBlock() const;
    void         setServerBlock(ServerBlock *);

    Location *getLocation(void);
    void      setLocation(Location *);

    Client *getClient();
    void          setClient(Client *);

    const std::string                       &getPath() const;
    const std::string                       &getMethod() const;
    const std::string                       &getProtocol() const;
    const std::map<uint32_t, RequestHeader> &getHeaders() const;
    const std::string                       &getBody() const;
    const uint8_t                           &getFlags() const;
    const HTTP::StatusCode                  &getStatus() const;
    URI                                     &getUriRef();
    URI                                     &getHostRef();
    URI                                     &getReferrerRef();
    const std::string                       &getRawUri() const;

    RangeList                               &getRangeList();

    bool    isFormed(void) const;
    void    isFormed(bool formed);

    // Needed to be improved
    const std::string &getQueryString() const;
    const std::string &getResolvedPath() const;
    const std::string  getHeaderValue(uint32_t key) const;

    bool empty();

    int  set(uint8_t flag) const;
    void setFlag(uint8_t flag);
    void removeFlag(uint8_t flag);
    void clear(void);

    bool parseLine(std::string &line);

    StatusCode parseSL(const std::string &line);
    StatusCode parseHeader(const std::string &line);
    StatusCode parseChunk(const std::string &line);
    StatusCode writeChunkSize(const std::string &line);
    StatusCode writeChuck(const std::string &line);
    StatusCode parseBody(const std::string &line);
    StatusCode writeBody(const std::string &line);

    StatusCode checkSL(void);
    StatusCode checkHeaders(void);
    void       resolvePath(void);

private:
    // bool isValidMethod(const std::string &method);
    // bool isValidPath(const std::string &path);
    bool isValidProtocol(const std::string &protocol);

public:
    // for chunked
    bool          getBodySizeFlag();
    void          setBodySizeFlag(bool isSize);
    size_t        getBodySize();
    void          setBodySize(size_t size);
    void          setStatus(const HTTP::StatusCode &status);

    bool authNeeded(void);
    bool isAuthorized(void) const;
    bool isHeaderExist(const HeaderCode code);
    bool isHeaderExist(const uint32_t code);
    void setAuthFlag(bool);

    uint32_t getStoredHash() const;
    void     setStoredHash(uint32_t);

    const std::map<std::string, std::string> &getCookie(void);
    void                                      setCookie(std::map<std::string, std::string> cookie);

    void chuckedRequested(bool);
    bool chuckedRequested(void);
};

} // namespace HTTP
