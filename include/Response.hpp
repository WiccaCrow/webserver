#pragma once

#include <cstdio>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <map>
#include <deque>
#include <list>
#include <sstream>
#include <stdio.h>
#include <cmath>
#include <stdlib.h>
#include <unistd.h>
#include <utility>
#include <sys/stat.h>
#include <sys/mman.h>

#include "SHA1.hpp"
#include "Request.hpp"
#include "ResponseHeader.hpp"
#include "HTML.hpp"

namespace HTTP {

# define CHUNK_SIZE 40960

class Client;

class Response {

    std::string _res;
    std::string _resLeftToSend;
    Request    *_req;
    Client     *_client;
    CGI        *_cgi;
    std::string _body;
    std::string _head;
    std::string _extraHeaders;
    size_t      _bodyLength;
    bool        _isFormed;
    StatusCode  _status;

    char        *_fileaddr;
    int         _filefd;
    struct stat _filestat;
    RangeSet    _range;

public:
    typedef std::list<ResponseHeader>::iterator iter;
    typedef std::list<ResponseHeader>::const_iterator const_iter;
    std::list<ResponseHeader> headers;

    Response(void);
    Response(Request *req);
    ~Response(void);

    Response(const Response &other);
    Response &operator=(const Response &other);

    void handle(void);

    //  for errors
    static const std::map<std::string, std::string> MIMEs;

    // methods
    void DELETE(void);
    void HEAD(void);
    void GET(void);
    void OPTIONS(void);
    void POST(void);
    void PUT(void);
    void CONNECT(void);
    void PATCH(void);
    void TRACE(void);

    void        performMethod(void);
    void        makeResponseForError(void);
    void        makeResponseForNonAuth(void);
    int         makeResponseForDir(void);
    int         makeResponseForFile(void);
    void        makeResponseForRange(void);
    void        makeResponseForMultipartRange(void);
    int         makeResponseForCGI(void);
    int         makeResponseForRedirect(StatusCode, const std::string &);

    int         contentForGetHead(void);
    bool        indexFileExists(const std::string &resourcePath);
    int         openFileToResponse(const std::string &resourcePath);
    int         listing(const std::string &resourcePath);
    int         fillDirContent(std::deque<std::string> &, const std::string &directory);
    std::string createTableLine(const std::string &file);
    int         fillFileStat(const std::string &file, struct stat *st);
    std::string getContentType(const std::string &resourcePath);

    ResponseHeader *getHeader(uint32_t hash);
    void            makeHead(void);
    void            addHeader(uint32_t hash, const std::string &value);
    void            addHeader(uint32_t hash);

    bool  isCGI(const std::string &filepath);
    
    void  makeChunk();

    // setters, getters
    // to send response
    void shouldBeClosedIf(void);

    const std::string &getBody(void) const;
    const std::string &getHead(void) const;
    void               setBody(const std::string &);
    size_t             getBodyLength(void) const;
    void               setBodyLength(size_t);
    Request           *getRequest(void);
    const std::string &getStatusLine(void);
    StatusCode         getStatus();
    void               setStatus(HTTP::StatusCode status);

    Client *        getClient(void);

    bool            isFormed(void) const;
    void            isFormed(bool formed);

    std::string getEtagFile(const std::string &filename);

    void *getFileAddr(void);
    int64_t getFileSize(void);

    const std::string getContentRangeValue(RangeSet &);
};

} // namespace HTTP
