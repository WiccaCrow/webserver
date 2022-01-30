#pragma once

#include <Utils.hpp>
#include <map>

#include "StatusCodes.hpp"

namespace HTTP {

class Response {
    std::string        _res;
    static std::string _bodyStyle;

    public:
    Response();
    ~Response() {}

    static const std::map<int, const char *> _ErrorCode;

    const char *                       findErr(int nbErr);
    static std::map<int, const char *> initErrorCode();

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
};

}; // namespace HTTP
