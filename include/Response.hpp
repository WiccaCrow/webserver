#pragma once

#include <Utils.hpp>
#include <unistd.h>
#include <dirent.h>
#include <StatusCodes.hpp>
#include <Request.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <cstdio>

namespace HTTP {
// # define SIZE_FOR_CHUNKED 4096
class Response {
    std::string     _res;
    std::string     _resLeftToSend;    
    bool            _responseFormed;

    public:
    Response();
    ~Response();

    void        clear();

    //  for errors

    static const std::map<std::string, std::string>     _ContType;
    static const std::map<int, const char *>            _ErrorCode;

    const char *                                findErr(int nbErr);
    static std::map<int, const char *>          initErrorCode();
    static std::map<std::string, std::string>   initContType();

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
    std::string contentForGetHead(Request &req);
    std::string resoursePathTaker(Request &req);
    std::string GetContentType(std::string resourcePath);
    std::string fileToResponse(std::string &resourcePath);
    std::string listToResponse(std::string &resourcePath, Request &req);
    void        POSTmethod(Request &req);

    void        doCGI(Request &req);

    // to send response
    size_t      GetResSize(void);
    const char  *GetResponse(void);
    const char  *GetLeftToSend(void);
    size_t      GetLeftToSendSize(void);
    void        SetLeftToSend(size_t n);

    // std::string        TransferEncodingChunked(std::string buffer, size_t bufSize);
};

}; // namespace HTTP
