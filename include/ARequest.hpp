#pragma once

#include <string>
#include <limits.h>

#include "Status.hpp"
#include "Logger.hpp"

namespace HTTP {

# define PARSED_NONE    0
# define PARSED_SL      1
# define PARSED_HEADERS 2
# define PARSED_BODY    4

class ARequest {

private:
    std::string       _protocol;
    int               _major : 4;
    int               _minor : 4;

    std::string       _body;
    std::string       _head;

    // Parsing
    bool              _isChunkSize;
    int64_t           _bodySize;
    std::size_t       _chunkSize;
    std::size_t       _parseFlags;

    bool              _headSent;
    bool              _bodySent;
    bool              _formed;
    bool              _chunked;
    bool              _isProxy;
    bool              _isCGI;

    // Internal status
    StatusCode        _status;

public:
    ARequest(void);
    virtual ~ARequest(void);

    // ARequest(const ARequest &);
    // ARequest &operator=(const ARequest &);
    
    void setMajor(int);
    void setMinor(int);
    void setStatus(StatusCode);
    void setFlag(std::size_t);
    void setBodySize(int64_t);
    void setHead(const std::string &);
    void setBody(const std::string &);
    void setProtocol(const std::string &);

    int getMajor(void) const;
    int getMinor(void) const;
    StatusCode getStatus(void) const;
    std::size_t getFlags(void) const;
    int64_t getBodySize(void) const;
    const std::string &getBody(void) const;
    const std::string &getHead(void) const;
    const std::string &getProtocol(void) const;

    bool flagSet(std::size_t) const;
    
    bool sent(void) const;
    void sent(bool);

    bool headSent(void) const;
    void headSent(bool);

    bool bodySent(void) const;
    void bodySent(bool);

    bool formed(void) const;
    void formed(bool);

    bool chunked(void) const;
    void chunked(bool);

    bool isChunkSize(void) const;
    void isChunkSize(bool);

    bool isProxy(void);
    void isProxy(bool);

    bool isCGI(void);
    void isCGI(bool);

    StatusCode parseChunk(const std::string &);
    StatusCode writeChunkSize(const std::string &);
    StatusCode writeChunk(const std::string &);

    virtual bool tunnelGuard(bool) = 0;

    virtual bool parseLine(std::string &) = 0;
    virtual StatusCode parseSL(const std::string &) = 0;
    virtual StatusCode checkSL(void) = 0;
    virtual StatusCode parseHeader(const std::string &) = 0;
    virtual StatusCode checkHeaders(void) = 0;    
    virtual StatusCode parseBody(const std::string &) = 0;
    virtual StatusCode writeBody(const std::string &) = 0;
};

}
