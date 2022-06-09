#include "Status.hpp"

namespace HTTP {

StatusLines::StatusLines() {
    // 1xx informational response
    _statusLines[CONTINUE]            = "HTTP/1.1 100 Continue\r\n";
    _statusLines[SWITCHING_PROTOCOLS] = "HTTP/1.1 101 Switching Protocols\r\n";
    _statusLines[PROCESSING]          = "HTTP/1.1 102 Processing\r\n";
    _statusLines[EARLY_HINTS]         = "HTTP/1.1 103 Early Hints\r\n";
    // 2xx success
    _statusLines[OK]                            = "HTTP/1.1 200 OK\r\n";
    _statusLines[CREATED]                       = "HTTP/1.1 201 Created\r\n";
    _statusLines[ACCEPTED]                      = "HTTP/1.1 202 Accepted\r\n";
    _statusLines[NON_AUTHORITATIVE_INFORMATION] = "HTTP/1.1 203 Non-Authoritative Information\r\n";
    _statusLines[NO_CONTENT]                    = "HTTP/1.1 204 No Content\r\n";
    _statusLines[RESET_CONTENT]                 = "HTTP/1.1 205 Reset Content\r\n";
    _statusLines[PARTIAL_CONTENT]               = "HTTP/1.1 206 Partial Content\r\n";
    _statusLines[MULTI_STATUS]                  = "HTTP/1.1 207 Multi-Status\r\n";
    _statusLines[ALREADY_REPORTED]              = "HTTP/1.1 208 Already Reported\r\n";
    _statusLines[IM_USED]                       = "HTTP/1.1 226 IM Used\r\n";
    // 3xx redirection
    _statusLines[MULTIPLE_CHOICES]   = "HTTP/1.1 300 Multiple Choices\r\n";
    _statusLines[MOVED_PERMANENTLY]  = "HTTP/1.1 301 Moved Permanently\r\n";
    _statusLines[FOUND]              = "HTTP/1.1 302 Found\r\n";
    _statusLines[SEE_OTHER]          = "HTTP/1.1 303 See Other\r\n";
    _statusLines[NOT_MODIFIED]       = "HTTP/1.1 304 Not Modified\r\n";
    _statusLines[USE_PROXY]          = "HTTP/1.1 305 Use Proxy\r\n";
    _statusLines[TEMPORARY_REDIRECT] = "HTTP/1.1 307 Temporary Redirect\r\n";
    _statusLines[PERMANENT_REDIRECT] = "HTTP/1.1 308 Permanent Redirect\r\n";
    // 4xx client errors
    _statusLines[BAD_REQUEST]                     = "HTTP/1.1 400 Bad Request\r\n";
    _statusLines[UNAUTHORIZED]                    = "HTTP/1.1 401 Unauthorized\r\n";
    _statusLines[PAYMENT_REQUIRED]                = "HTTP/1.1 402 Payment Required\r\n";
    _statusLines[FORBIDDEN]                       = "HTTP/1.1 403 Forbidden\r\n";
    _statusLines[NOT_FOUND]                       = "HTTP/1.1 404 Not Found\r\n";
    _statusLines[METHOD_NOT_ALLOWED]              = "HTTP/1.1 405 Method Not Allowed\r\n";
    _statusLines[NOT_ACCEPTABLE]                  = "HTTP/1.1 406 Not Acceptable\r\n";
    _statusLines[PROXY_AUTHENTICATION_REQUIRED]   = "HTTP/1.1 407 Proxy Authentication Required\r\n";
    _statusLines[REQUEST_TIMEOUT]                 = "HTTP/1.1 408 Request Timeout\r\n";
    _statusLines[CONFLICT]                        = "HTTP/1.1 409 Conflict\r\n";
    _statusLines[GONE]                            = "HTTP/1.1 410 Gone\r\n";
    _statusLines[LENGTH_REQUIRED]                 = "HTTP/1.1 411 Length Required\r\n";
    _statusLines[PRECONDITION_FAILED]             = "HTTP/1.1 412 Precondition Failed\r\n";
    _statusLines[PAYLOAD_TOO_LARGE]               = "HTTP/1.1 413 Payload Too Large\r\n";
    _statusLines[URI_TOO_LONG]                    = "HTTP/1.1 414 URI Too Long\r\n";
    _statusLines[UNSUPPORTED_MEDIA_TYPE]          = "HTTP/1.1 415 Unsupported Media Type\r\n";
    _statusLines[RANGE_NOT_SATISFIABLE]           = "HTTP/1.1 416 Range Not Satisfiable\r\n";
    _statusLines[EXPECTATION_FAILED]              = "HTTP/1.1 417 Expectation Failed\r\n";
    _statusLines[IM_A_TEAPOT]                     = "HTTP/1.1 418 I'm a teapot\r\n";
    _statusLines[AUTHENTICATION_TIMEOUT]          = "HTTP/1.1 419 Authentication Timeout\r\n";
    _statusLines[MISDIRECTED_REQUEST]             = "HTTP/1.1 421 Misdirected Request\r\n";
    _statusLines[UNPROCESSABLE_ENTITY]            = "HTTP/1.1 422 Unprocessable Entity\r\n";
    _statusLines[LOCKED]                          = "HTTP/1.1 423 Locked\r\n";
    _statusLines[FAILED_DEPENDENCY]               = "HTTP/1.1 424 Failed Dependency\r\n";
    _statusLines[TOO_EARLY]                       = "HTTP/1.1 425 Too Early\r\n";
    _statusLines[UPGRADE_REQUIRED]                = "HTTP/1.1 426 Upgrade Required\r\n";
    _statusLines[PRECONDITION_REQUIRED]           = "HTTP/1.1 428 Precondition Required\r\n";
    _statusLines[TOO_MANY_REQUESTS]               = "HTTP/1.1 429 Too Many Requests\r\n";
    _statusLines[REQUEST_HEADER_FIELDS_TOO_LARGE] = "HTTP/1.1 431 Request Header Fields Too Large\r\n";
    _statusLines[RETRY_WITH]                      = "HTTP/1.1 449 Retry With\r\n";
    _statusLines[UNAVAILABLE_FOR_LEGAL_REASONS]   = "HTTP/1.1 451 Unavailable For Legal Reasons\r\n";
    _statusLines[CLIENT_CLOSED_REQUEST]           = "HTTP/1.1 499 Client Closed Request\r\n";
    // 5xx server errors
    _statusLines[INTERNAL_SERVER_ERROR]           = "HTTP/1.1 500 Internal Server Error\r\n";
    _statusLines[NOT_IMPLEMENTED]                 = "HTTP/1.1 501 Not Implemented\r\n";
    _statusLines[BAD_GATEWAY]                     = "HTTP/1.1 502 Bad Gateway\r\n";
    _statusLines[SERVICE_UNAVAILABLE]             = "HTTP/1.1 503 Service Unavailable\r\n";
    _statusLines[GATEWAY_TIMEOUT]                 = "HTTP/1.1 504 Gateway Timeout\r\n";
    _statusLines[HTTP_VERSION_NOT_SUPPORTED]      = "HTTP/1.1 505 HTTP Version Not Supported\r\n";
    _statusLines[VARIANT_ALSO_NEGOTIATES]         = "HTTP/1.1 506 Variant Also Negotiates\r\n";
    _statusLines[INSUFFICIENT_STORAGE]            = "HTTP/1.1 507 Insufficient Storage\r\n";
    _statusLines[LOOP_DETECTED]                   = "HTTP/1.1 508 Loop Detected\r\n";
    _statusLines[BANDWIDTH_LIMIT_EXCEEDED]        = "HTTP/1.1 509 Bandwidth Limit Exceeded\r\n";
    _statusLines[NOT_EXTENDED]                    = "HTTP/1.1 510 Not Extended\r\n";
    _statusLines[NETWORK_AUTHENTICATION_REQUIRED] = "HTTP/1.1 511 Network Authentication Required\r\n";
    // Cloudflare ext
    _statusLines[UNKNOWN_ERROR]           = "HTTP/1.1 520 Web Server Returned an Unknown Error\r\n";
    _statusLines[WEB_SERVER_IS_DOWN]      = "HTTP/1.1 521 Web Server Is Down\r\n";
    _statusLines[CONNECTION_TIMED_OUT]    = "HTTP/1.1 522 Connection Timed Out\r\n";
    _statusLines[ORIGIN_IS_UNREACHABLE]   = "HTTP/1.1 523 Origin Is Unreachable\r\n";
    _statusLines[A_TIMEOUT_OCCURRED]      = "HTTP/1.1 524 A Timeout Occurred\r\n";
    _statusLines[SSL_HANDSHAKE_FAILED]    = "HTTP/1.1 525 SSL Handshake Failed\r\n";
    _statusLines[INVALID_SSL_CERTIFICATE] = "HTTP/1.1 526 Invalid SSL Certificate\r\n";
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