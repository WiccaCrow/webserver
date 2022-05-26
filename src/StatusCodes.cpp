#include "StatusCodes.hpp"

namespace HTTP {

StatusDescription::StatusDescription() {
    // 1xx informational response
    _descriptions[CONTINUE] = "Continue";
    _descriptions[SWITCHING_PROTOCOLS] = "Switching Protocols";
    _descriptions[PROCESSING] = "Processing";
    _descriptions[EARLY_HINTS] = "Early Hints";
    // 2xx success
    _descriptions[OK] = "OK";
    _descriptions[CREATED] = "Created";
    _descriptions[ACCEPTED] = "Accepted";
    _descriptions[NON_AUTHORITATIVE_INFORMATION] = "Non-Authoritative Information";
    _descriptions[NO_CONTENT] = "No Content";
    _descriptions[RESET_CONTENT] = "Reset Content";
    _descriptions[PARTIAL_CONTENT] = "Partial Content";
    _descriptions[MULTI_STATUS] = "Multi-Status";
    _descriptions[ALREADY_REPORTED] = "Already Reported";
    _descriptions[IM_USED] = "IM Used";
    // 3xx redirection
    _descriptions[MULTIPLE_CHOICES] = "Multiple Choices";
    _descriptions[MOVED_PERMANENTLY] = "Moved Permanently";
    _descriptions[FOUND] = "Found";
    _descriptions[SEE_OTHER] = "See Other";
    _descriptions[NOT_MODIFIED] = "Not Modified";
    _descriptions[USE_PROXY] = "Use Proxy";
    _descriptions[TEMPORARY_REDIRECT] = "Temporary Redirect";
    _descriptions[PERMANENT_REDIRECT] = "Permanent Redirect";
    // 4xx client errors
    _descriptions[BAD_REQUEST] = "Bad Request";
    _descriptions[UNAUTHORIZED] = "Unauthorized";
    _descriptions[PAYMENT_REQUIRED] = "Payment Required";
    _descriptions[FORBIDDEN] = "Forbidden";
    _descriptions[NOT_FOUND] = "Not Found";
    _descriptions[METHOD_NOT_ALLOWED] = "Method Not Allowed";
    _descriptions[NOT_ACCEPTABLE] = "Not Acceptable";
    _descriptions[PROXY_AUTHENTICATION_REQUIRED] = "Proxy Authentication Required";
    _descriptions[REQUEST_TIMEOUT] = "Request Timeout";
    _descriptions[CONFLICT] = "Conflict";
    _descriptions[GONE] = "Gone";
    _descriptions[LENGTH_REQUIRED] = "Length Required";
    _descriptions[PRECONDITION_FAILED] = "Precondition Failed";
    _descriptions[PAYLOAD_TOO_LARGE] = "Payload Too Large";
    _descriptions[URI_TOO_LONG] = "URI Too Long";
    _descriptions[UNSUPPORTED_MEDIA_TYPE] = "Unsupported Media Type";
    _descriptions[RANGE_NOT_SATISFIABLE] = "Range Not Satisfiable";
    _descriptions[EXPECTATION_FAILED] = "Expectation Failed";
    _descriptions[IM_A_TEAPOT] = "I'm a teapot";
    _descriptions[AUTHENTICATION_TIMEOUT] = "Authentication Timeout";
    _descriptions[MISDIRECTED_REQUEST] = "Misdirected Request";
    _descriptions[UNPROCESSABLE_ENTITY] = "Unprocessable Entity";
    _descriptions[LOCKED] = "Locked";
    _descriptions[FAILED_DEPENDENCY] = "Failed Dependency";
    _descriptions[TOO_EARLY] = "Too Early";
    _descriptions[UPGRADE_REQUIRED] = "Upgrade Required";
    _descriptions[PRECONDITION_REQUIRED] = "Precondition Required";
    _descriptions[TOO_MANY_REQUESTS] = "Too Many Requests";
    _descriptions[REQUEST_HEADER_FIELDS_TOO_LARGE] = "Request Header Fields Too Large";
    _descriptions[RETRY_WITH] = "Retry With";
    _descriptions[UNAVAILABLE_FOR_LEGAL_REASONS] = "Unavailable For Legal Reasons";
    _descriptions[CLIENT_CLOSED_REQUEST] = "Client Closed Request";
    // 5xx server errors
    _descriptions[INTERNAL_SERVER_ERROR] = "Internal Server Error";
    _descriptions[NOT_IMPLEMENTED] = "Not Implemented";
    _descriptions[BAD_GATEWAY] = "Bad Gateway";
    _descriptions[SERVICE_UNAVAILABLE] = "Service Unavailable";
    _descriptions[GATEWAY_TIMEOUT] = "Gateway Timeout";
    _descriptions[HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
    _descriptions[VARIANT_ALSO_NEGOTIATES] = "Variant Also Negotiates";
    _descriptions[INSUFFICIENT_STORAGE] = "Insufficient Storage";
    _descriptions[LOOP_DETECTED] = "Loop Detected";
    _descriptions[BANDWIDTH_LIMIT_EXCEEDED] = "";
    _descriptions[NOT_EXTENDED] = "Not Extended";
    _descriptions[NETWORK_AUTHENTICATION_REQUIRED] = "Network Authentication Required";
    // Cloudflare ext
    _descriptions[UNKNOWN_ERROR] = "Web Server Returned an Unknown Error";
    _descriptions[WEB_SERVER_IS_DOWN] = "Web Server Is Down";
    _descriptions[CONNECTION_TIMED_OUT] = "Connection Timed Out";
    _descriptions[ORIGIN_IS_UNREACHABLE] = "Origin Is Unreachable";
    _descriptions[A_TIMEOUT_OCCURRED] = "A Timeout Occurred";
    _descriptions[SSL_HANDSHAKE_FAILED] = "SSL Handshake Failed";
    _descriptions[INVALID_SSL_CERTIFICATE] = "Invalid SSL Certificate";
}

StatusDescription::~StatusDescription() {}

const std::string &StatusDescription::operator[](StatusCode status) {
    return _descriptions[status];
}

const std::string &StatusDescription::operator[](int code) {
    return _descriptions[(StatusCode)code];
}

const StatusDescription descriptions;

};