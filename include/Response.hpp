#pragma once

#include <Utils.hpp>


namespace HTTP {

class Response {
    std::string res;

    public:
    Response() {}
    ~Response() {}

    const char *getData(void) {
        std::string body = "<html><body> Hello from the other side! </body></html>";
        res += "HTTP/1.1 200 OK\r\n";
        res += "Content-type: text/html; charset=utf-8\r\n";
        res += "Connection: keep-alive\r\n";
        res += "Keep-Alive: timeout=55, max=1000\r\n";
        res += "Content-length: " + to_string(body.length()) + "\r\n\r\n";
        res += body;

        return res.c_str();
    }
    // Errors:
    const char *    ErrorCli400(void);
    const char *    ErrorCli404(std::string HTTPvers);

    // const char *    ErrorCli405(void);
    // const char *    ErrorCli406(void);
    // const char *    ErrorCli408(void);
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

};

}; // namespace HTTP
