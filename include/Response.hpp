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
#include "ErrorResponses.hpp"

namespace HTTP {

class Client;

class Response : public ARequest {

    std::string _rawStatus;

    Request    *_req;
    CGI        *_cgi;
    bool        _pending;

    char        *_fileaddr;
    int         _filefd;
    struct stat _filestat;

    RangeSet    _range;

    bool        _isProxy;
    bool        _isCGI;

public:
    Headers<ResponseHeader> headers;

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

    void        makeResponseForMethod(void);
    void        makeResponseForError(void);
    void        makeResponseForNonAuth(void);
    int         makeResponseForDir(void);
    int         makeResponseForFile(void);
    void        makeResponseForRange(void);
    void        makeResponseForMultipartRange(void);
    int         makeResponseForCGI(void);
    int         makeResponseForRedirect(StatusCode, const std::string &);

    int         contentForGetHead(void);
    bool        indexFileExists(const std::string &);
    int         openFileToResponse(const std::string &);
    int         listing(const std::string &);
    int         fillDirContent(std::deque<std::string> &, const std::string &directory);
    std::string createTableLine(const std::string &);
    int         fillFileStat(const std::string &file, struct stat *st);
    std::string getContentType(const std::string &);

    void        makeHead(void);
    void        addHeader(uint32_t, const std::string & = "");

    void        matchCGI(const std::string &filepath);
    
    void  makeChunk();

    // setters, getters
    // to send response
    void shouldBeClosedIf(void);

    const std::string &getStatusLine(void);

    Request *getRequest(void);
    Client *getClient(void);

    bool pending(void) const;
    void pending(bool);

    std::string getEtagFile(const std::string &filename);

    void *getFileAddr(void);
    int64_t getFileSize(void);

    bool isCGI(void) const;
    bool isProxy(void) const;

    const std::string getContentRangeValue(RangeSet &);

    virtual bool parseLine(std::string &);
    virtual StatusCode parseSL(const std::string &);
    virtual StatusCode checkSL(void);
    virtual StatusCode parseHeader(const std::string &);
    virtual StatusCode checkHeaders(void);    
    virtual StatusCode parseBody(const std::string &);
    virtual StatusCode writeBody(const std::string &);

};

} // namespace HTTP
