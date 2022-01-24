#pragma once

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
};

}; // namespace HTTP
