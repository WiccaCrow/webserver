#include "Response.hpp"
#include "ErrorResponses.hpp"

namespace HTTP {

ErrorResponses::ErrorResponses() {
    _errorResponses.insert(std::make_pair(HTTP::BAD_REQUEST,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[BAD_REQUEST] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG + statusLines[BAD_REQUEST] + H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::UNAUTHORIZED,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[UNAUTHORIZED] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[UNAUTHORIZED] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::FORBIDDEN,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[FORBIDDEN] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[FORBIDDEN] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::NOT_FOUND,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[NOT_FOUND] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[NOT_FOUND] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::METHOD_NOT_ALLOWED,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[METHOD_NOT_ALLOWED] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[METHOD_NOT_ALLOWED] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::NOT_ACCEPTABLE,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[NOT_ACCEPTABLE] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[NOT_ACCEPTABLE] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::REQUEST_TIMEOUT,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[REQUEST_TIMEOUT] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[REQUEST_TIMEOUT] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::LENGTH_REQUIRED,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[LENGTH_REQUIRED] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[LENGTH_REQUIRED] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::PAYLOAD_TOO_LARGE,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[PAYLOAD_TOO_LARGE] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[PAYLOAD_TOO_LARGE] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::URI_TOO_LONG,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[URI_TOO_LONG] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[URI_TOO_LONG] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::UNSUPPORTED_MEDIA_TYPE,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[UNSUPPORTED_MEDIA_TYPE] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[UNSUPPORTED_MEDIA_TYPE] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::INTERNAL_SERVER_ERROR,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[INTERNAL_SERVER_ERROR] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[INTERNAL_SERVER_ERROR] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::NOT_IMPLEMENTED,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[NOT_IMPLEMENTED] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[NOT_IMPLEMENTED] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::BAD_GATEWAY,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[BAD_GATEWAY] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[BAD_GATEWAY] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::GATEWAY_TIMEOUT,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[GATEWAY_TIMEOUT] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[GATEWAY_TIMEOUT] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::HTTP_VERSION_NOT_SUPPORTED,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[HTTP_VERSION_NOT_SUPPORTED] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[HTTP_VERSION_NOT_SUPPORTED] + B_END H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::UNKNOWN_ERROR,
        HTML_BEG HEAD_BEG TITLE_BEG + statusLines[UNKNOWN_ERROR] + TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG B_BEG + statusLines[UNKNOWN_ERROR] + B_END H1_CENTER_END HR BODY_END HTML_END));
}

ErrorResponses::~ErrorResponses() {}

bool ErrorResponses::has(int code) const {
    return _errorResponses.find(code) != _errorResponses.end();
}

bool ErrorResponses::has(StatusCode code) const {
    return this->has(static_cast<int>(code));
}

const std::string &ErrorResponses::operator[](StatusCode code) const {
    return this->operator[](static_cast<int>(code));
}

const std::string &ErrorResponses::operator[](int code) const {
    std::map<int, std::string>::const_iterator it = _errorResponses.find(code);
    if (it == _errorResponses.end()) {
        return _errorResponses.find(UNKNOWN_ERROR)->second;
    }
    return it->second;
}

const ErrorResponses errorResponses;

}