#include "StatusCodes.hpp"

namespace HTTP {

static std::map<StatusCode, std::string>
initDescriptions() {
    std::map<StatusCode, std::string> descriptions;
    // 1xx informational response
    descriptions[CONTINUE]            = "HTTP/1.1 100 Continue";
    descriptions[SWITCHING_PROTOCOLS] = "HTTP/1.1 101 Switching Protocols";
    descriptions[PROCESSING]          = "HTTP/1.1 102 Processing";
    descriptions[EARLY_HINTS]         = "HTTP/1.1 103 Early Hints";
    // 2xx success
    descriptions[OK]                            = "HTTP/1.1 200 OK";
    descriptions[CREATED]                       = "HTTP/1.1 201 Created";
    descriptions[ACCEPTED]                      = "HTTP/1.1 202 Accepted";
    descriptions[NON_AUTHORITATIVE_INFORMATION] = "HTTP/1.1 203 Non-Authoritative Information";
    descriptions[NO_CONTENT]                    = "HTTP/1.1 204 No Content";
    descriptions[RESET_CONTENT]                 = "HTTP/1.1 205 Reset Content";
    descriptions[PARTIAL_CONTENT]               = "HTTP/1.1 206 Partial Content";
    descriptions[MULTI_STATUS]                  = "HTTP/1.1 207 Multi-Status";
    descriptions[ALREADY_REPORTED]              = "HTTP/1.1 208 Already Reported";
    descriptions[IM_USED]                       = "HTTP/1.1 226 IM Used";
    // 3xx redirection
    descriptions[MULTIPLE_CHOICES]   = "HTTP/1.1 300 Multiple Choices";
    descriptions[MOVED_PERMANENTLY]  = "HTTP/1.1 301 Moved Permanently";
    descriptions[FOUND]              = "HTTP/1.1 302 Found";
    descriptions[SEE_OTHER]          = "HTTP/1.1 303 See Other";
    descriptions[NOT_MODIFIED]       = "HTTP/1.1 304 Not Modified";
    descriptions[USE_PROXY]          = "HTTP/1.1 305 Use Proxy";
    descriptions[TEMPORARY_REDIRECT] = "HTTP/1.1 307 Temporary Redirect";
    descriptions[PERMANENT_REDIRECT] = "HTTP/1.1 308 Permanent Redirect";
    // 4xx client errors
    descriptions[BAD_REQUEST]                     = "HTTP/1.1 400 Bad Request";
    descriptions[UNAUTHORIZED]                    = "HTTP/1.1 401 Unauthorized";
    descriptions[PAYMENT_REQUIRED]                = "HTTP/1.1 402 Payment Required";
    descriptions[FORBIDDEN]                       = "HTTP/1.1 403 Forbidden";
    descriptions[NOT_FOUND]                       = "HTTP/1.1 404 Not Found";
    descriptions[METHOD_NOT_ALLOWED]              = "HTTP/1.1 405 Method Not Allowed";
    descriptions[NOT_ACCEPTABLE]                  = "HTTP/1.1 406 Not Acceptable";
    descriptions[PROXY_AUTHENTICATION_REQUIRED]   = "HTTP/1.1 407 Proxy Authentication Required";
    descriptions[REQUEST_TIMEOUT]                 = "HTTP/1.1 408 Request Timeout";
    descriptions[CONFLICT]                        = "HTTP/1.1 409 Conflict";
    descriptions[GONE]                            = "HTTP/1.1 410 Gone";
    descriptions[LENGTH_REQUIRED]                 = "HTTP/1.1 411 Length Required";
    descriptions[PRECONDITION_FAILED]             = "HTTP/1.1 412 Precondition Failed";
    descriptions[PAYLOAD_TOO_LARGE]               = "HTTP/1.1 413 Payload Too Large";
    descriptions[URI_TOO_LONG]                    = "HTTP/1.1 414 URI Too Long";
    descriptions[UNSUPPORTED_MEDIA_TYPE]          = "HTTP/1.1 415 Unsupported Media Type";
    descriptions[RANGE_NOT_SATISFIABLE]           = "HTTP/1.1 416 Range Not Satisfiable";
    descriptions[EXPECTATION_FAILED]              = "HTTP/1.1 417 Expectation Failed";
    descriptions[IM_A_TEAPOT]                     = "HTTP/1.1 418 I'm a teapot";
    descriptions[AUTHENTICATION_TIMEOUT]          = "HTTP/1.1 419 Authentication Timeout";
    descriptions[MISDIRECTED_REQUEST]             = "HTTP/1.1 421 Misdirected Request";
    descriptions[UNPROCESSABLE_ENTITY]            = "HTTP/1.1 422 Unprocessable Entity";
    descriptions[LOCKED]                          = "HTTP/1.1 423 Locked";
    descriptions[FAILED_DEPENDENCY]               = "HTTP/1.1 424 Failed Dependency";
    descriptions[TOO_EARLY]                       = "HTTP/1.1 425 Too Early";
    descriptions[UPGRADE_REQUIRED]                = "HTTP/1.1 426 Upgrade Required";
    descriptions[PRECONDITION_REQUIRED]           = "HTTP/1.1 428 Precondition Required";
    descriptions[TOO_MANY_REQUESTS]               = "HTTP/1.1 429 Too Many Requests";
    descriptions[REQUEST_HEADER_FIELDS_TOO_LARGE] = "HTTP/1.1 431 Request Header Fields Too Large";
    descriptions[RETRY_WITH]                      = "HTTP/1.1 449 Retry With";
    descriptions[UNAVAILABLE_FOR_LEGAL_REASONS]   = "HTTP/1.1 451 Unavailable For Legal Reasons";
    descriptions[CLIENT_CLOSED_REQUEST]           = "HTTP/1.1 499 Client Closed Request";
    // 5xx server errors
    descriptions[INTERNAL_SERVER_ERROR]           = "HTTP/1.1 500 Internal Server Error";
    descriptions[NOT_IMPLEMENTED]                 = "HTTP/1.1 501 Not Implemented";
    descriptions[BAD_GATEWAY]                     = "HTTP/1.1 502 Bad Gateway";
    descriptions[SERVICE_UNAVAILABLE]             = "HTTP/1.1 503 Service Unavailable";
    descriptions[GATEWAY_TIMEOUT]                 = "HTTP/1.1 504 Gateway Timeout";
    descriptions[HTTP_VERSION_NOT_SUPPORTED]      = "HTTP/1.1 505 HTTP Version Not Supported";
    descriptions[VARIANT_ALSO_NEGOTIATES]         = "HTTP/1.1 506 Variant Also Negotiates";
    descriptions[INSUFFICIENT_STORAGE]            = "HTTP/1.1 507 Insufficient Storage";
    descriptions[LOOP_DETECTED]                   = "HTTP/1.1 508 Loop Detected";
    descriptions[BANDWIDTH_LIMIT_EXCEEDED]        = "HTTP/1.1 509 Bandwidth Limit Exceeded";
    descriptions[NOT_EXTENDED]                    = "HTTP/1.1 510 Not Extended";
    descriptions[NETWORK_AUTHENTICATION_REQUIRED] = "HTTP/1.1 511 Network Authentication Required";
    // Cloudflare ext
    descriptions[UNKNOWN_ERROR]           = "HTTP/1.1 520 Web Server Returned an Unknown Error";
    descriptions[WEB_SERVER_IS_DOWN]      = "HTTP/1.1 521 Web Server Is Down";
    descriptions[CONNECTION_TIMED_OUT]    = "HTTP/1.1 522 Connection Timed Out";
    descriptions[ORIGIN_IS_UNREACHABLE]   = "HTTP/1.1 523 Origin Is Unreachable";
    descriptions[A_TIMEOUT_OCCURRED]      = "HTTP/1.1 524 A Timeout Occurred";
    descriptions[SSL_HANDSHAKE_FAILED]    = "HTTP/1.1 525 SSL Handshake Failed";
    descriptions[INVALID_SSL_CERTIFICATE] = "HTTP/1.1 526 Invalid SSL Certificate";
}

const std::map<StatusCode, std::string> descriptions = initDescriptions();

};