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
    ~Response() {}

    static const std::map<std::string, std::string>     _ContType;
    static const std::map<int, const char *>            _ErrorCode;

    const char *                                findErr(int nbErr);
    static std::map<int, const char *>          initErrorCode();
    static std::map<std::string, std::string>   initContType();

    // const char *getData(void) {
    //     std::string body = "<html><body> Hello from the other side! </body></html>";
    //     _res = "HTTP/1.1 200 OK\r\n";
    //     _res += "Content-type: text/html; charset=utf-8\r\n";
    //     _res += "Connection: keep-alive\r\n";
    //     _res += "Keep-Alive: timeout=55, max=1000\r\n";
    //     _res += "Content-length: " + to_string(body.length()) + "\r\n\r\n";
    //     _res += body;

    //     return _res.c_str();
    // }

    bool isFormed() const {
        return _responseFormed;
    }

    void setFormed(bool formed) {
        _responseFormed = formed;
    }

    // void resetResponse() {
    //     _res = "";
    // }

    // Errors:
    // 400 403 404 405 408 411 413 414 415
    // 500 501 502 504 505
    // const char *    ErrorCli406(void);
    // const char *    ErrorCli409(void);
    // const char *    ErrorCli410(void);
    // const char *    ErrorCli412(void); // ??
    // const char *    ErrorServ503(void);

    std::string doCGI(Request &req);

    void        HEADmethod(Request &req);
    void        GETmethod(Request &req);
    void        DELETEmethod(Request &req);
    std::string contentForGetHead(Request &req);
    std::string resoursePathTaker(Request &req);
    std::string GetContentType(std::string resourcePath);
    std::string fileToResponse(std::string &resourcePath);
    std::string listToResponse(std::string &resourcePath, Request &req);

    void        POSTmethod(Request &req);

    size_t      GetResSize(void);
    const char  *GetResponse(void);
    const char  *GetLeftToSend(void);
    size_t      GetLeftToSendSize(void);
    
    void        SetLeftToSend(size_t n);

    // std::string        TransferEncodingChunked(std::string buffer, size_t bufSize);
};

}; // namespace HTTP
