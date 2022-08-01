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
#include <ctime>

#include "SHA1.hpp"
#include "Request.hpp"
#include "ResponseHeader.hpp"
#include "HTML.hpp"
#include "ErrorResponses.hpp"
#include "Cookie.hpp"
#include "Globals.hpp"
#include "ETag.hpp"

namespace HTTP {

class Client;

class Response : public ARequest {

    std::string _rawStatus;
    StatusCode  _parsedStatus;

    Request    *_req;
    CGI        *_cgi;
    Proxy      *_proxy;

    RangeSet    _range;

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

    void        assembleError(void);
    void        makeResponseForMethod(void);
    void        makeResponseForProxy(void);
    void        makeResponseForError(void);
    void        makeResponseForNonAuth(void);
    int         makeResponseForDir(void);
    int         makeResponseForFile(void);
    int         makeResponseForRange(void);
    int         makeResponseForMultipartRange(void);
    int         makeResponseForCGI(void);
    int         makeResponseForRedirect(StatusCode, const std::string &);

    void        makeFileWithRandName(const std::string &directory);

    int         contentForGetHead(void);
    bool        indexFileExists(const std::string &);
    int         openFile(const std::string &);
    int         listing(const std::string &);
    int         fillDirContent(std::deque<std::string> &, const std::string &directory);
    std::string createTableLine(const std::string &);
    int         fillFileStat(const std::string &file, struct stat *st);
    std::string getContentType(const std::string &);

    void        makeHead(void);
    void        addHeader(uint32_t, const std::string & = "");

    void        matchCGI(const std::string &filepath);

    void        checkCGIFailure(void);
    
    virtual bool has(uint32_t hash);

    // setters, getters
    // to send response

    const std::string &getStatusLine(void);

    void setCGI(CGI *);
    CGI *getCGI(void) const;

    Request *getRequest(void);

    void *getFileAddr(void);
    int64_t getFileSize(void);

    const std::string getContentRangeValue(RangeSet &);

    virtual bool parseLine(std::string &);
    virtual StatusCode parseSL(const std::string &);
    virtual StatusCode checkSL(void);
    virtual StatusCode parseHeader(const std::string &);
    virtual StatusCode checkHeaders(void);    

    bool writeBodyToFile(const std::string &);
};

} // namespace HTTP
