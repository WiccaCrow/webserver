#include "Response.hpp"

const std::map<int, const char*> 
HTTP::Response::_ErrorCode = HTTP::Response::initErrorCode();

std::map<int, const char*> HTTP::Response::initErrorCode() {
    std::map<int, const char*> Err;
    Err.insert(std::make_pair(BAD_REQUEST,
                              "HTTP/1.1 400 Bad Request\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 550\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 400 (Bad Request)</title>\n"
                              "<p><b>400.</b> That's an error.\n"
                              "<p><err_text>Invalid or illegal request.</err_text>"));
    Err.insert(std::make_pair(FORBIDDEN,
                              "HTTP/1.1 403 Forbidden\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 573\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 403 (Forbidden)</title>\n"
                              "<p><b>403.</b> Forbidden.\n"
                              "<p><err_text>You may not have sufficient rights to perform the action..</err_text>"));
    Err.insert(std::make_pair(NOT_FOUND,
                              "HTTP/1.1 404 Not Found\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 564\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 404 (Not Found)</title>\n"
                              "<p><b>404.</b> Page not found.\n"
                              "<p><err_text>Not found anything matching the Request-URI.</err_text>"));
    Err.insert(std::make_pair(METHOD_NOT_ALLOWED,
                              "HTTP/1.1 405 Method Not Allowed\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 676\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 405 (Method Not Allowed)</title>\n"
                              "<p><b>405.</b> The method not supported by the target resource."
                              "<p><err_text> The method received in the request-line is known by the "
                              "<p>origin server but not supported by the target resource.</err_text>"));
    Err.insert(std::make_pair(REQUEST_TIMEOUT,
                              "HTTP/1.1 408 Timeout\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 626\r\n"
                              "Connection: close\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 408 (Request Timeout)</title>\n"
                              "<p><b>408.</b> Request Timeout.\n"
                              "<p><err_text>The server did not receive a complete request message within "
                              "the time that it was prepared to wait.</err_text>"));
    Err.insert(std::make_pair(LENGTH_REQUIRED,
                              "HTTP/1.1 411 Length Required\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 620\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 411 (Length Required)</title>\n"
                              "<p><b>411.</b> Length Required.\n"
                              "<p><err_text>For the specified resource, the client must "
                              "specify the Content-Length in the request header.</err_text>"));
    Err.insert(std::make_pair(PAYLOAD_TOO_LARGE,
                              "HTTP/1.1 413 Payload Too Large\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 561\r\n"
                              "Connection: close\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 413 (Payload Too Large)</title>\n"
                              "<p><b>413.</b> Payload Too Large.\n"
                              "<p><err_text>The request body is too large.</err_text>"));
    Err.insert(std::make_pair(URI_TOO_LONG,
                              "HTTP/1.1 414 URI Too Long\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 572\r\n"
                              "Connection: close\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 414 (URI Too Long)</title>\n"
                              "<p><b>414.</b> URI Too Long.\n"
                              "<p><err_text>The URI in the request is too long "
                              "for this server.</err_text>"));
    Err.insert(std::make_pair(UNSUPPORTED_MEDIA_TYPE,
                              "HTTP/1.1 415 Unsupported Media Type\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 584\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 415 (Unsupported Media Type)</title>\n"
                              "<p><b>415.</b> Unsupported Media Type.\n"
                              "<p><err_text>Content format not supported by the server."
                              "</err_text>"));
    Err.insert(std::make_pair(INTERNAL_SERVER_ERROR,
                              "HTTP/1.1 500 Internal Server Error\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 602\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 500 (Internal Server Error)</title>\n"
                              "<p><b>500.</b> Internal Server Error.\n"
                              "<p><err_text>An unexpected error prevented the "
                              "request from being completed.</err_text>"));
    Err.insert(std::make_pair(NOT_IMPLEMENTED,
                              "HTTP/1.1 501 Not Implemented\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 553\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 501 (Not Implemented)</title>\n"
                              "<p><b>501.</b> Not Implemented."
                              "<p><err_text> Unknown method in request.</err_text>"));
    Err.insert(std::make_pair(BAD_GATEWAY,
                              "HTTP/1.1 502 Bad Gateway\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 532\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 502 (Bad Gateway)</title>\n"
                              "<p><b>502.</b> Bad Gateway."
                              "<p><err_text>It happens...</err_text>"));
    Err.insert(std::make_pair(GATEWAY_TIMEOUT,
                              "HTTP/1.1 504 Gateway Timeout\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 622\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 504 (Gateway Timeout)</title>\n"
                              "<p><b>504.</b> Gateway Timeout."
                              "<p><err_text>The server did not wait for a "
                              "response from the upstream server to complete "
                              "the current request.</err_text>"));
    Err.insert(std::make_pair(HTTP_VERSION_NOT_SUPPORTED,
                              "HTTP/1.1 505 HTTP Version Not Supported\r\n"
                              "Content-Type: text/html; charset=UTF-8\r\n"
                              "Referrer-Policy: no-referrer\r\n"
                              "Content-length: 575\r\n"
                              "\r\n"
                              "<!DOCTYPE html>\n"
                              "<html lang=en>\n"
                              "<meta charset=utf-8>\n"
                              "<style>\n"
                              "*{margin:0;padding:0}html{font:15px/22px arial,"
                              "sans-serif}html{background:#fff;color:#222;"
                              "padding:15px}body{margin:7\% auto 0;max-width:"
                              "390px;min-height:180px;padding:30px 0 15px}p"
                              "{margin:11px 0 22px;overflow:hidden}err_text"
                              "{color:#777;text-decoration:none}"
                              "@media screen and (max-width:772px){body{background:none;"
                              "margin-top:0;max-width:none;padding-right:0}}\n"
                              "</style>\n"
                              "<title>Error 505 (HTTP Version Not Supported)</title>\n"
                              "<p><b>505.</b> HTTP Version Not Supported."
                              "<p><err_text>HTTP version not supported.</err_text>"));
    return (Err);
}