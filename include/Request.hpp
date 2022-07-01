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

#define PARSED_NONE    0
#define PARSED_SL      1
#define PARSED_HEADERS 2
#define PARSED_BODY    4

namespace HTTP {

class Client;

class ARequest {

private:
    std::string       _protocol;
    int               _major : 4;
    int               _minor : 4;

    std::string       _body;
    std::string       _head;

    
    // Parsing
    bool              _isChunkSize;
    std::size_t            _bodySize;
    std::size_t            _chunkSize;
    std::size_t            _parseFlags;

    bool              _formed;
    bool              _chunked;

    // Internal status
    StatusCode        _status;

public:
    ARequest(void);
    virtual ~ARequest(void);

    ARequest(const ARequest &);
    ARequest &operator=(const ARequest &);
    
    void setProtocol(const std::string &protocol);
    void setMajor(int major);
    void setMinor(int minor);
    void setFlag(std::size_t);
    void setBody(const std::string &);
    void setHead(const std::string &);
    void setBodySize(std::size_t);
    void setStatus(StatusCode);

    bool flagSet(std::size_t) const;
    std::size_t getFlags(void) const;
    std::size_t getBodySize(void) const;
    StatusCode getStatus(void) const;
    const std::string &getBody(void) const;
    const std::string &getHead(void) const;
    const std::string &getProtocol(void) const;
    int getMajor(void);
    int getMinor(void);
    
    StatusCode parseChunk(const std::string &);
    StatusCode writeChunkSize(const std::string &);
    StatusCode writeChuck(const std::string &);

    virtual bool parseLine(std::string &) = 0;
    virtual StatusCode parseSL(const std::string &) = 0;
    virtual StatusCode checkSL(void) = 0;
    virtual StatusCode parseHeader(const std::string &) = 0;
    virtual StatusCode checkHeaders(void) = 0;    
    virtual StatusCode parseBody(const std::string &) = 0;
    virtual StatusCode writeBody(const std::string &) = 0;

    bool formed(void) const;
    void formed(bool);

    bool chunked(void) const;
    void chunked(bool);
    bool isChunkSize(void) const;
    void isChunkSize(bool);

};






class Request : public ARequest {

private:
    std::string                       _method;
    std::string                       _rawURI;

    URI                               _uri;
    URI                               _host;
    URI                               _referrer;

    RangeList                         _ranges;

    std::string                       _resolvedPath;
    
    ServerBlock *                     _servBlock;
    Location    *                     _location;
    Client      *                     _client;


    // why here?
    uint32_t                          _storedHash; 
    bool                              _authorized;


    std::map<std::string, std::string> _cookie; // ?

public:
    Headers<RequestHeader>            headers;

    Request(void);
    Request(Client *);
    ~Request(void);

    Request(const Request &other);
    Request &operator=(const Request &other);

    ServerBlock *getServerBlock() const;
    void         setServerBlock(ServerBlock *);

    Location *getLocation(void) const;
    void      setLocation(Location *);

    Client *getClient(void);
    void    setClient(Client *);


    virtual bool parseLine(std::string &);

    virtual StatusCode parseSL(const std::string &);
    virtual StatusCode checkSL(void);

    virtual StatusCode parseHeader(const std::string &);
    virtual StatusCode checkHeaders(void);

    virtual StatusCode parseBody(const std::string &);
    virtual StatusCode writeBody(const std::string &);

    const std::string    &getPath(void) const;
    const std::string    &getMethod(void) const;
    const std::string    &getRawUri(void) const;
    
    URI                  &getUriRef(void);
    URI                  &getHostRef(void);
    URI                  &getReferrerRef(void);

    RangeList            &getRangeList(void);

    const std::string    &getResolvedPath(void) const;
    void setResolvedPath(const std::string &);
    void resolvePath(void);
    
    bool authorized(void) const;
    void authorized(bool);

    uint32_t getStoredHash() const;
    void     setStoredHash(uint32_t);

    const std::map<std::string, std::string> &getCookie(void);
    void                                      setCookie(std::map<std::string, std::string> cookie);
};

} // namespace HTTP
