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
#include <fstream>

#include "Request.hpp"
#include "StatusCodes.hpp"
#include "Utils.hpp"

namespace HTTP {
// # define SIZE_FOR_CHUNKED 4096
class Response {

    std::string _res;
    std::string _resLeftToSend;
    bool        _responseFormed;

public:
    Response();
    ~Response();

    void clear();

    //  for errors
    static const std::map<std::string, std::string> MIMEs;
    static const std::map<HTTP::StatusCode, const char *>  errorResponses;

    void setErrorResponse(HTTP::StatusCode status);

    // These functions allow you to get / set
    // a flag by which you can find out about
    // the need to form a response (the request
    // is fully processed)

    bool isFormed() const;
    void setFormed(bool formed);

    // methods

    void        HEADmethod(Request &req);
    void        GETmethod(Request &req);
    void        DELETEmethod(Request &req);
    void        POSTmethod(Request &req);
    void        PUTmethod(Request &req);
    std::string contentForGetHead(Request &req);
    std::string getContentType(std::string resourcePath);
    std::string fileToResponse(std::string resourcePath);
    std::string listing(const std::string &resourcePath, Request &req);
    void        createFile(Request &req, const std::string &resourcePath);

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
