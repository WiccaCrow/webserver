#include "Response.hpp"

std::string HTTP::Response::_bodyStyle =
    "<!DOCTYPE html>\n\
                <html lang=en>\n\
                <meta charset=utf-8>\n";

HTTP::Response::Response() {
}

void HTTP::Response::SetBodyStyle() {
    _bodyStyle +=
        "<style>\n\
                    *{margin:0;padding:0}html{font:15px/22px arial,\
                    sans-serif}html{background:#fff;color:#222;\
                    padding:15px}body{margin:7\% auto 0;max-width:\
                    390px;min-height:180px;padding:30px 0 15px}p\
                    {margin:11px 0 22px;overflow:hidden}err_text\
                    {color:#777;text-decoration:none}\
                    @media screen and (max-width:772px){body{background:none;\
                    margin-top:0;max-width:none;padding-right:0}}\n\
                </style>\n";
}

const char* HTTP::Response::ErrorCli400(void) {
    std::string body = _bodyStyle +
                       "<title>Error 400 (Bad Request)</title>\n\
    <p><b>400.</b> That's an error.\n\
    <p><err_text>Invalid or illegal request.</err_text>";
    _res = "HTTP/1.0 400 Bad Request\r\n";
    _res += "Content-Type: text/html; charset=UTF-8\r\n";
    _res += "Referrer-Policy: no-referrer\r\n";
    _res += "Content-length: " + to_string(body.length()) + "\r\n";
    _res += "\r\n";
    _res += body;
    return _res.c_str();
}

const char* HTTP::Response::ErrorCli404(std::string protocol) {
    std::string body =
        "<title>Error 404 (Not Found)</title>\n\
    <p><b>404.</b> Page not found.\n\
    <p><err_text>Not found anything matching the Request-URI.</err_text>";
    _res = protocol;
    _res += " 404 Not Found\r\n";
    _res += "Content-Type: text/html; charset=UTF-8\r\n";
    _res += "Referrer-Policy: no-referrer\r\n";
    _res += "Content-length: " + to_string(body.length()) + "\r\n";
    _res += "\r\n";
    _res += body;
    return _res.c_str();
}

const char* HTTP::Response::ErrorCli405(std::string protocol) {
    std::string body =
        "<title>Error 405 (Not Found)</title>\n\
    <p><b>404.</b> Page not found.\n\
    <p><err_text>Not found anything matching the Request-URI.</err_text>";
    _res = protocol;
    _res += " 404 Not Found\r\n";
    _res += "Content-Type: text/html; charset=UTF-8\r\n";
    _res += "Referrer-Policy: no-referrer\r\n";
    _res += "Content-length: " + to_string(body.length()) + "\r\n";
    _res += "\r\n";
    _res += body;
    return _res.c_str();
}
const char* HTTP::Response::ErrorCli408(std::string protocol) {
    std::string body =
        "<title>Error 408 (Request Timeout)</title>\n\
    <p><b>404.</b> Page not found.\n\
    <p><err_text>The server did not receive a complete request message within \
    <p>the time that it was   prepared to wait.</err_text>";
    _res = protocol;
    _res += " 408 Timeout\r\n";
    _res += "Content-Type: text/html; charset=UTF-8\r\n";
    _res += "Referrer-Policy: no-referrer\r\n";
    _res += "Content-length: " + to_string(body.length()) + "\r\n";
    _res += "Connection: close\r\n";
    _res += "\r\n";
    _res += body;
    return _res.c_str();
}

// Errors:

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
