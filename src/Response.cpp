#include "Response.hpp"

const char *    HTTP::Response::ErrorCli400(void) {
    std::string body = "<!DOCTYPE html>\n\
    <html lang=en>\n\
    <meta charset=utf-8>\n\
    <title>Error 400 (Bad Request)</title>\n\
    <style>\n\
        *{margin:0;padding:0}html{font:15px/22px arial,\
        sans-serif}html{background:#fff;color:#222;\
        padding:15px}body{margin:7\% auto 0;max-width:\
        390px;min-height:180px;padding:30px 0 15px}p\
        {margin:11px 0 22px;overflow:hidden}err_text\
        {color:#777;text-decoration:none}\
        @media screen and (max-width:772px){body{background:none;\
        margin-top:0;max-width:none;padding-right:0}}\n\
    </style>\n\
    <p><b>400.</b> That's an error.\n\
    <p><err_text>Invalid or illegal request.</err_text>";
    res = "HTTP/1.0 400 Bad Request\r\n";
    res += "Content-Type: text/html; charset=UTF-8\r\n";
    res += "Referrer-Policy: no-referrer\r\n";
    res += "Content-length: " + to_string(body.length()) + "\r\n";
    // res += Date() + "\r\n";
    res += "\r\n";

    res += body;

    return res.c_str();
}

const char *    HTTP::Response::ErrorCli404(std::string protocol) {
    std::string body = "<!DOCTYPE html>\n\
    <html lang=en>\n\
    <meta charset=utf-8>\n\
    <title>Error 404 (Not Found)</title>\n\
    <style>\n\
        *{margin:0;padding:0}html{font:15px/22px arial,\
        sans-serif}html{background:#fff;color:#222;\
        padding:15px}body{margin:7\% auto 0;max-width:\
        390px;min-height:180px;padding:30px 0 15px}p\
        {margin:11px 0 22px;overflow:hidden}err_text\
        {color:#777;text-decoration:none}\
        @media screen and (max-width:772px){body{background:none;\
        margin-top:0;max-width:none;padding-right:0}}\n\
    </style>\n\
    <p><b>404.</b> Page not found.\n\
    <p><err_text>Not found anything matching the Request-URI.</err_text>";
    res = protocol;
    res += " 404 Not Found\r\n";
    res += "Content-Type: text/html; charset=UTF-8\r\n";
    res += "Referrer-Policy: no-referrer\r\n";
    res += "Content-length: " + to_string(body.length()) + "\r\n";
    // res += Date() + "\r\n";
    res += "\r\n";

    res += body;

    return res.c_str();
}
// HTTP/1.0 400 Bad Request
// Content-Type: text/html; charset=UTF-8
// Referrer-Policy: no-referrer
// Content-Length: 1555
// Date: Thu, 27 Jan 2022 23:00:47 GMT


    // Errors:
    
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
