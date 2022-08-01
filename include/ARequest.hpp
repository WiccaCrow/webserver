#pragma once

#include <string>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "Globals.hpp"
#include "Status.hpp"
#include "Logger.hpp"
#include "Header.hpp"

namespace HTTP {

# define PARSED_NONE    0
# define PARSED_SL      1
# define PARSED_HEADERS 2
# define PARSED_BODY    4

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
    int64_t           _expBodySize;
    int64_t           _realBodySize;
    std::size_t       _chunkSize;
    std::size_t       _parseFlags;

    bool              _headSent;
    bool              _bodySent;
    bool              _formed;
    bool              _chunked;
    bool              _isProxy;
    bool              _isCGI;
    bool              _parted;

    // Internal status
    StatusCode        _status;

    Client          * _client;

protected:
    std::string      _filename;
    char           * _fileaddr;
    int              _filefd;
    struct stat      _filestat;
    uint64_t         _offset;

public:
    ARequest(void);
    virtual ~ARequest(void);

    void setMajor(int);
    void setMinor(int);
    void setStatus(StatusCode);
    void setFlag(std::size_t);
    void setExpBodySize(int64_t);
    void setRealBodySize(int64_t);
    void setHead(const std::string &);
    void setBody(const std::string &);
    void appendBody(const std::string &);
    void setProtocol(const std::string &);

    int getMajor(void) const;
    int getMinor(void) const;
    StatusCode getStatus(void) const;
    std::size_t getFlags(void) const;
    int64_t getExpBodySize(void) const;
    int64_t getRealBodySize(void) const;
    const std::string &getBody(void) const;
    const std::string &getHead(void) const;
    const std::string &getProtocol(void) const;
    
    Client *getClient(void);
    void setClient(Client *);

    int getFileFd(void) const;
    const std::string &getFilename(void) const;

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

    bool parted(void) const;
    void parted(bool);

    bool isChunkSize(void) const;
    void isChunkSize(bool);

    bool isProxy(void);
    void isProxy(bool);

    bool isCGI(void);
    void isCGI(bool);

    bool tunnelGuard(bool);

    virtual bool has(uint32_t hdrhash) = 0;

    virtual bool parseLine(std::string &) = 0;
    virtual StatusCode parseSL(const std::string &) = 0;
    virtual StatusCode checkSL(void) = 0;
    virtual StatusCode parseHeader(const std::string &) = 0;
    virtual StatusCode checkHeaders(void) = 0;
    
    StatusCode parseBody(const std::string &);
    StatusCode writeBody(const std::string &);
    StatusCode writeChunkSize(const std::string &);
    StatusCode writeChunk(const std::string &);
    StatusCode writePart(const std::string &);
    
    std::string makeChunk(void);
    std::string makePart(void);

    bool createTmpFile(void);
    bool mapFile(void);

};

}
