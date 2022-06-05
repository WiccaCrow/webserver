#pragma once

#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <utility>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>

#include "Request.hpp"
#include "Status.hpp"
#include "Utils.hpp"

namespace HTTP {
// # define SIZE_FOR_CHUNKED 4096
class Response {


    std::string _res;
    std::string _resLeftToSend;

public:

    typedef void  (Response::*Handler)(Request &req);
    std::map<std::string, Handler> methods;

    Response();
    ~Response();

    void clear();

    StatusCode handle(Request &req);

    //  for errors
    static const std::map<std::string, std::string> MIMEs;
    static const std::map<HTTP::StatusCode, const char *>  errorResponses;

    void setErrorResponse(HTTP::StatusCode status);

    // methods

    void        DELETE(Request &req);
    void        HEAD(Request &req);
    void        GET(Request &req);
    void        OPTIONS(Request &req);
    void        POST(Request &req);
    void        PUT(Request &req);
    // void        generateHeaders(Request &req);
    std::string contentForGetHead(Request &req);
    std::string getContentType(std::string resourcePath);
    std::string fileToResponse(std::string resourcePath);
    std::string listing(const std::string &resourcePath, Request &req);
    void        writeFile(Request &req, const std::string &resourcePath);

    std::string passToCGI(Request &req, CGI &cgi);

    // to send response
    size_t      getResSize(void);
    const char *getResponse(void);
    const char *getLeftToSend(void);
    size_t      getLeftToSendSize(void);
    void        setLeftToSend(size_t n);

    // std::string        TransferEncodingChunked(std::string buffer, size_t bufSize);
};

} // namespace HTTP
