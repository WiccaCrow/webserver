#pragma once

#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utility>

#include "Request.hpp"
#include "Status.hpp"
#include "Utils.hpp"

namespace HTTP {
// # define SIZE_FOR_CHUNKED 4096
class Response {

    std::string _res;
    std::string _resLeftToSend;
    Request    *_req;
    std::string _body;
    bool _shouldBeClosed;

public:
    typedef void (Response::*Handler)();
    std::map<std::string, Handler> methods;

    Response();
    ~Response();

    void clear();

    StatusCode handle(Request &req);

    //  for errors
    static const std::map<std::string, std::string>       MIMEs;
    static const std::map<HTTP::StatusCode, const char *> errorResponses;

    void setErrorResponse(HTTP::StatusCode status);

    // methods

    void DELETE(void);
    void HEAD(void);
    void GET(void);
    void OPTIONS(void);
    void POST(void);
    void PUT(void);
    // void        generateHeaders(Request &req);
    std::string contentForGetHead(void);
    std::string getContentType(std::string resourcePath);
    std::string fileToResponse(std::string resourcePath);
    std::string listing(const std::string &resourcePath);
    void        writeFile(const std::string &resourcePath);

    std::string passToCGI(CGI &cgi);

    // to send response
    size_t             getResLength(void);
    const char        *getResponse(void);
    const char        *getLeftToSend(void);
    size_t             getLeftToSendSize(void);
    const std::string &getBody(void) const;
    void               setLeftToSend(size_t n);
    void               setRequest(Request *req);
    Request           *getRequest(void) const;

    bool            shouldBeClosed(void) const;
    void            shouldBeClosed(bool);

    // std::string        TransferEncodingChunked(std::string buffer, size_t bufSize);
};

} // namespace HTTP
