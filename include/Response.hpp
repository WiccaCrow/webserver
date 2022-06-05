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

typedef void  (Response::*methodFunction)(Request &req);

    // std::vector<std::pair<std::string, methodFunction> > Methods;

    std::string _res;
    std::string _resLeftToSend;

public:
    std::map<std::string, methodFunction> Methods;
    
    Response();
    ~Response();

    void clear();

    //  for errors
    static const std::map<std::string, std::string> MIMEs;
    static const std::map<HTTP::StatusCode, const char *>  errorResponses;

    void setErrorResponse(HTTP::StatusCode status);

    // methods

    void        DELETEmethod(Request &req);
    void        HEADmethod(Request &req);
    void        GETmethod(Request &req);
    void        OPTIONSmethod(Request &req);
    void        POSTmethod(Request &req);
    void        PUTmethod(Request &req);
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
