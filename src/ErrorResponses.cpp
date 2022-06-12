#include "Response.hpp"
#include "ErrorResponses.hpp"

namespace HTTP {

std::string
readFile(const std::string &filename) {
    std::string res;

    std::ifstream in(filename.c_str());
    if (!in.is_open()) {
        return "";
    }
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

int
HTTP::Response::setErrorResponse(HTTP::StatusCode status) {

    std::map<int, std::string> &pages = getRequest()->getServerBlock()->getErrPathsRef();
    std::map<int, std::string>::iterator it = pages.find(status);

    std::string response;
    if (it != pages.end()) {
        response = readFile(it->second);
    
        if (!response.empty()) {
            setStatus(status);
            setBody(response);
            return 0;
        }
    }

    response = errorResponses[status];
    if (response.empty()) {
        Log.error("Unknown response error code: " + to_string(static_cast<int>(status)));
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
        "<!DOCTYPE html><html><head><title>400 Bad Request</title></head>"
        "<body><center><h1><b>400</b> Bad Request</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::FORBIDDEN,
        "<!DOCTYPE html><html><head><title>403 Forbidden</title></head>"
        "<body><center><h1><b>403</b> Forbidden</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::NOT_FOUND,
        "<!DOCTYPE html><html><head><title>404 Page not found</title></head>"
        "<body><center><h1><b>404</b> Page not found</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::METHOD_NOT_ALLOWED,
        "<!DOCTYPE html><html><head><title>405 Method Not Allowed</title></head>"
        "<body><center><h1><b>405</b> Method Not Allowed</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::REQUEST_TIMEOUT,
        "<!DOCTYPE html><html><head><title>408 Request Timeout</title></head>"
        "<body><center><h1><b>408</b> Request Timeout</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::LENGTH_REQUIRED,
        "<!DOCTYPE html><html><head><title>411 Length Required</title></head>"
        "<body><center><h1><b>411</b> Length Required</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::PAYLOAD_TOO_LARGE,
        "<!DOCTYPE html><html><head><title>413 Payload Too Large</title></head>"
        "<body><center><h1><b>413</b> Payload Too Large</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::URI_TOO_LONG,
        "<!DOCTYPE html><html><head><title>414 URI Too Long</title></head>"
        "<body><center><h1><b>414</b> URI Too Long</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::UNSUPPORTED_MEDIA_TYPE,
        "<!DOCTYPE html><html><head><title>415 Unsupported Media Type</title></head>"
        "<body><center><h1><b>415</b> Unsupported Media Type</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::INTERNAL_SERVER_ERROR,
        "<!DOCTYPE html><html><head><title>500 Internal Server Error</title></head>"
        "<body><center><h1><b>500</b> Internal Server Error</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::NOT_IMPLEMENTED,
        "<!DOCTYPE html><html><head><title>501 Not Implemented</title></head>"
        "<body><center><h1><b>501</b> Not Implemented</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::BAD_GATEWAY,
        "<!DOCTYPE html><html><head><title>502 Bad Gateway</title></head>"
        "<body><center><h1><b>502</b> Bad Gateway</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::GATEWAY_TIMEOUT,
        "<!DOCTYPE html><html><head><title>504 Gateway Timeout</title></head>"
        "<body><center><h1><b>504</b> Gateway Timeout</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(HTTP::HTTP_VERSION_NOT_SUPPORTED,
        "<!DOCTYPE html><html><head><title>505 HTTP Version Not Supported</title></head>"
        "<body><center><h1>505 HTTP Version Not Supported</h1></center><hr></body></html>"));
    _errorResponses.insert(std::make_pair(66,
        "<!DOCTYPE html><html><head><title>XXX Unknown response status code</title></head>"
        "<body><center><h1><b>XXX</b> Unknown response status code</h1></center><hr></body></html>"));
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