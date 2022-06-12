#include "Response.hpp"
#include "ErrorResponses.hpp"

namespace HTTP {

int
HTTP::Response::setErrorResponse(HTTP::StatusCode status) {
    const std::string &response = errorResponses[status];

    // if serverblock has error-pages, but response doesn't have a reference to serverblock
    // std::map<int, std::string>::iterator it = getRequest()->getServerBlock()->getErrPathsRef();
    if (response.empty()) {
        Log.error("Unknown response error code: " + to_string(static_cast<int>(status)));
        // Should be removed after development
        setStatus(INTERNAL_SERVER_ERROR);
        setBody(errorResponses[66]);
    } else {
        setStatus(status);
        setBody(response);
    }
    return 0;
}

ErrorResponses::ErrorResponses() {
    _errorResponses.insert(std::make_pair(HTTP::BAD_REQUEST,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 400 (Bad Request)</title>"
        "<p><b>400.</b> That's an error</p>"
        "<err_text>Invalid or illegal request.</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::FORBIDDEN,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 403 (Forbidden)</title>"
        "<p><b>403.</b> Forbidden</p>"
        "<err_text>You may not have sufficient rights to perform the action...</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::NOT_FOUND,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 404 (Not Found)</title>"
        "<p><b>404.</b> Page not found</p>"
        "<err_text>Not found anything matching the Request-URI.</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::METHOD_NOT_ALLOWED,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 405 (Method Not Allowed)</title>"
        "<p><b>405.</b> The method not supported by the target resource</p>"
        "<err_text> The method received in the request-line is known by the "
        "origin server but not supported by the target resource.</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::REQUEST_TIMEOUT,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 408 (Request Timeout)</title>"
        "<p><b>408.</b> Request Timeout</p>"
        "<err_text>The server did not receive a complete request message within "
        "the time that it was prepared to wait.</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::LENGTH_REQUIRED,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 411 (Length Required)</title>"
        "<p><b>411.</b> Length Required</p>"
        "<err_text>For the specified resource, the client must "
        "specify the Content-Length in the request header.</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::PAYLOAD_TOO_LARGE,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 413 (Payload Too Large)</title>"
        "<p><b>413.</b> Payload Too Large</p>"
        "<err_text>The request body is too large.</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::URI_TOO_LONG,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 414 (URI Too Long)</title>"
        "<p><b>414.</b> URI Too Long</p>"
        "<err_text>The URI in the request is too long "
        "for this server.</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::UNSUPPORTED_MEDIA_TYPE,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 415 (Unsupported Media Type)</title>"
        "<p><b>415.</b> Unsupported Media Type</p>"
        "<err_text>Content format not supported by the server."
        "</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::INTERNAL_SERVER_ERROR,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 500 (Internal Server Error)</title>"
        "<p><b>500.</b> Internal Server Error</p>"
        "<err_text>An unexpected error prevented the "
        "request from being completed.</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::NOT_IMPLEMENTED,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 501 (Not Implemented)</title>"
        "<p><b>501.</b> Not Implemented</p>"
        "<err_text> Unknown method in request.</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::BAD_GATEWAY,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 502 (Bad Gateway)</title>"
        "<p><b>502.</b> Bad Gateway</p>"
        "<err_text>It happens...</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::GATEWAY_TIMEOUT,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 504 (Gateway Timeout)</title>"
        "<p><b>504.</b> Gateway Timeout</p>"
        "<err_text>The server did not wait for a "
        "response from the upstream server to complete "
        "the current request.</err_text>"));
    _errorResponses.insert(std::make_pair(HTTP::HTTP_VERSION_NOT_SUPPORTED,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error 505 (HTTP Version Not Supported)</title>"
        "<p><b>505.</b> HTTP Version Not Supported</p>"
        "<err_text>HTTP version not supported.</err_text>"));
    _errorResponses.insert(std::make_pair(66,
        ERROR_RESPONSE_BODY_BASIS
        "<title>Error ??? (Unknown response error code)</title>"
        "<p><b>???.</b> Unknown response error code.</p>"
        "<err_text>HTTP version not supported.</err_text>"));
}

ErrorResponses::~ErrorResponses() {}

const std::string &ErrorResponses::operator[](StatusCode code) const {
    return this->operator[](static_cast<int>(code));
}

const std::string &ErrorResponses::operator[](int code) const {
    std::map<int, std::string>::const_iterator it = _errorResponses.find(code);
    if (it == _errorResponses.end()) {
        return _empty;
    }
    return it->second;
}

const ErrorResponses errorResponses;

}