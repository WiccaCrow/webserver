#include "Status.hpp"

namespace HTTP {

StatusLines::StatusLines() {

    // 1xx informational response
    _statusLines[CONTINUE]            = "100 Continue";
    _statusLines[SWITCHING_PROTOCOLS] = "101 Switching Protocols";
    _statusLines[PROCESSING]          = "102 Processing";
    _statusLines[EARLY_HINTS]         = "103 Early Hints";
    
    // 2xx success
    _statusLines[OK]                            = "200 OK";
    _statusLines[CREATED]                       = "201 Created";
    _statusLines[ACCEPTED]                      = "202 Accepted";
    _statusLines[NON_AUTHORITATIVE_INFORMATION] = "203 Non-Authoritative Information";
    _statusLines[NO_CONTENT]                    = "204 No Content";
    _statusLines[RESET_CONTENT]                 = "205 Reset Content";
    _statusLines[PARTIAL_CONTENT]               = "206 Partial Content";
    _statusLines[MULTI_STATUS]                  = "207 Multi-Status";
    _statusLines[ALREADY_REPORTED]              = "208 Already Reported";
    _statusLines[IM_USED]                       = "226 IM Used";
    
    // 3xx redirection
    _statusLines[MULTIPLE_CHOICES]   = "300 Multiple Choices";
    _statusLines[MOVED_PERMANENTLY]  = "301 Moved Permanently";
    _statusLines[FOUND]              = "302 Found";
    _statusLines[SEE_OTHER]          = "303 See Other";
    _statusLines[NOT_MODIFIED]       = "304 Not Modified";
    _statusLines[USE_PROXY]          = "305 Use Proxy";
    _statusLines[TEMPORARY_REDIRECT] = "307 Temporary Redirect";
    _statusLines[PERMANENT_REDIRECT] = "308 Permanent Redirect";
    
    // 4xx client errors
    _statusLines[BAD_REQUEST]                     = "400 Bad Request";
    _statusLines[UNAUTHORIZED]                    = "401 Unauthorized";
    _statusLines[PAYMENT_REQUIRED]                = "402 Payment Required";
    _statusLines[FORBIDDEN]                       = "403 Forbidden";
    _statusLines[NOT_FOUND]                       = "404 Not Found";
    _statusLines[METHOD_NOT_ALLOWED]              = "405 Method Not Allowed";
    _statusLines[NOT_ACCEPTABLE]                  = "406 Not Acceptable";
    _statusLines[PROXY_AUTHENTICATION_REQUIRED]   = "407 Proxy Authentication Required";
    _statusLines[REQUEST_TIMEOUT]                 = "408 Request Timeout";
    _statusLines[CONFLICT]                        = "409 Conflict";
    _statusLines[GONE]                            = "410 Gone";
    _statusLines[LENGTH_REQUIRED]                 = "411 Length Required";
    _statusLines[PRECONDITION_FAILED]             = "412 Precondition Failed";
    _statusLines[PAYLOAD_TOO_LARGE]               = "413 Payload Too Large";
    _statusLines[URI_TOO_LONG]                    = "414 URI Too Long";
    _statusLines[UNSUPPORTED_MEDIA_TYPE]          = "415 Unsupported Media Type";
    _statusLines[RANGE_NOT_SATISFIABLE]           = "416 Range Not Satisfiable";
    _statusLines[EXPECTATION_FAILED]              = "417 Expectation Failed";
    _statusLines[IM_A_TEAPOT]                     = "418 I'm a teapot";
    _statusLines[AUTHENTICATION_TIMEOUT]          = "419 Authentication Timeout";
    _statusLines[MISDIRECTED_REQUEST]             = "421 Misdirected Request";
    _statusLines[UNPROCESSABLE_ENTITY]            = "422 Unprocessable Entity";
    _statusLines[LOCKED]                          = "423 Locked";
    _statusLines[FAILED_DEPENDENCY]               = "424 Failed Dependency";
    _statusLines[TOO_EARLY]                       = "425 Too Early";
    _statusLines[UPGRADE_REQUIRED]                = "426 Upgrade Required";
    _statusLines[PRECONDITION_REQUIRED]           = "428 Precondition Required";
    _statusLines[TOO_MANY_REQUESTS]               = "429 Too Many Requests";
    _statusLines[REQUEST_HEADER_FIELDS_TOO_LARGE] = "431 Request Header Fields Too Large";
    _statusLines[RETRY_WITH]                      = "449 Retry With";
    _statusLines[UNAVAILABLE_FOR_LEGAL_REASONS]   = "451 Unavailable For Legal Reasons";
    _statusLines[CLIENT_CLOSED_REQUEST]           = "499 Client Closed Request";
    
    // 5xx server errors
    _statusLines[INTERNAL_SERVER_ERROR]           = "500 Internal Server Error";
    _statusLines[NOT_IMPLEMENTED]                 = "501 Not Implemented";
    _statusLines[BAD_GATEWAY]                     = "502 Bad Gateway";
    _statusLines[SERVICE_UNAVAILABLE]             = "503 Service Unavailable";
    _statusLines[GATEWAY_TIMEOUT]                 = "504 Gateway Timeout";
    _statusLines[HTTP_VERSION_NOT_SUPPORTED]      = "505 HTTP Version Not Supported";
    _statusLines[VARIANT_ALSO_NEGOTIATES]         = "506 Variant Also Negotiates";
    _statusLines[INSUFFICIENT_STORAGE]            = "507 Insufficient Storage";
    _statusLines[LOOP_DETECTED]                   = "508 Loop Detected";
    _statusLines[BANDWIDTH_LIMIT_EXCEEDED]        = "509 Bandwidth Limit Exceeded";
    _statusLines[NOT_EXTENDED]                    = "510 Not Extended";
    _statusLines[NETWORK_AUTHENTICATION_REQUIRED] = "511 Network Authentication Required";
    
    // Cloudflare ext
    _statusLines[UNKNOWN_ERROR]           = "520 Web Server Returned an Unknown Error";
    _statusLines[WEB_SERVER_IS_DOWN]      = "521 Web Server Is Down";
    _statusLines[CONNECTION_TIMED_OUT]    = "522 Connection Timed Out";
    _statusLines[ORIGIN_IS_UNREACHABLE]   = "523 Origin Is Unreachable";
    _statusLines[A_TIMEOUT_OCCURRED]      = "524 A Timeout Occurred";
    _statusLines[SSL_HANDSHAKE_FAILED]    = "525 SSL Handshake Failed";
    _statusLines[INVALID_SSL_CERTIFICATE] = "526 Invalid SSL Certificate";
}

StatusLines::~StatusLines() {}

const std::string &StatusLines::operator[](StatusCode code) const {
    std::map<int, std::string>::const_iterator it = _statusLines.find(code);
    if (it == _statusLines.end()) {
        return _empty;
    }
    return it->second;
}

const std::string &StatusLines::operator[](int code) const {
    return this->operator[](static_cast<StatusCode>(code));
}

const StatusLines statusLines;

};