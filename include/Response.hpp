#pragma once

#include <Utils.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
// #include <vector>

#include "StatusCodes.hpp"

namespace HTTP {
# define SIZE_FOR_CHUNKED 4096
class Response {
    std::string     _res;
    bool            _responseFormed;
    // static std::string          _bodyStyle;

    public:
    Response();
    ~Response() {}

    static const std::map<std::string, std::string>     _ContType;
    static const std::map<int, const char *>            _ErrorCode;

    const char *                                findErr(int nbErr);
    static std::map<int, const char *>          initErrorCode();
    static std::map<std::string, std::string>   initContType();

    const char *getData(void) {
        std::string body = "<html><body> Hello from the other side! </body></html>";
        _res = "HTTP/1.1 200 OK\r\n";
        _res += "Content-type: text/html; charset=utf-8\r\n";
        _res += "Connection: keep-alive\r\n";
        _res += "Keep-Alive: timeout=55, max=1000\r\n";
        _res += "Content-length: " + to_string(body.length()) + "\r\n\r\n";
        _res += body;

        return _res.c_str();
    }

    bool isFormed() const {
        return _responseFormed;
    }

    void setFormed(bool formed) {
        _responseFormed = formed;
    }
    static void SetBodyStyle();
    // Errors:
    const char *ErrorCli400(void);
    const char *ErrorCli404(std::string HTTPvers);
    const char *ErrorCli405(std::string HTTPvers);
    const char *ErrorCli408(std::string protocol);

    // const char *    ErrorCli406(void);
    // const char *    ErrorCli409(void);
    // const char *    ErrorCli410(void);
    // const char *    ErrorCli411(void);
    // const char *    ErrorCli412(void); // ??
    // const char *    ErrorCli413(void);
    // const char *    ErrorCli414(void);
    // const char *    ErrorCli415(void);

    // const char *    ErrorServ500(void);
    // const char *    ErrorServ502(void);
    // const char *    ErrorServ503(void);
    // const char *    ErrorServ504(void);
    // const char *    ErrorServ505(void);
    const char* GETautoindexOn(std::string resourcePath);
    const char* GETautoindexOn_HtmlCss(std::string resourcePath);
    std::string GetContentType(std::string resourcePath);
    size_t  GetResSize();

    void TransferEncodingChunked(std::string buffer, size_t bufSize);
};

}; // namespace HTTP
