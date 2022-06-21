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

    std::string response;
    if (getRequest()->getServerBlock() != NULL) {

        std::map<int, std::string> &pages = getRequest()->getServerBlock()->getErrPathsRef();
        std::map<int, std::string>::iterator it = pages.find(status);

        if (it != pages.end()) {
            response = readFile(it->second);
        
            if (!response.empty()) {
                setStatus(status);
                setBody(response);
                return 0;
            }
        }
    }

    response = errorResponses[status];
    if (response.empty()) {
        Log.error() << "Unknown response code: " << static_cast<int>(status) << std::endl;
        setStatus(UNKNOWN_ERROR);
        setBody(errorResponses[UNKNOWN_ERROR]);
    } else {
        setStatus(status);
        setBody(response);
    }
    return 0;
}

ErrorResponses::ErrorResponses() {
    _errorResponses.insert(std::make_pair(HTTP::BAD_REQUEST,
        HTML_BEG HEAD_BEG TITLE_BEG "400 Bad Request" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>400</b> Bad Request" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::FORBIDDEN,
        HTML_BEG HEAD_BEG TITLE_BEG "403 Forbidden" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>403</b> Forbidden" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::NOT_FOUND,
        HTML_BEG HEAD_BEG TITLE_BEG "404 Page not found" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>404</b> Page not found" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::METHOD_NOT_ALLOWED,
        HTML_BEG HEAD_BEG TITLE_BEG "405 Method Not Allowed" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>405</b> Method Not Allowed" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::NOT_ACCEPTABLE,
        HTML_BEG HEAD_BEG TITLE_BEG "406 Not acceptable" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>406</b> Not acceptable" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::REQUEST_TIMEOUT,
        HTML_BEG HEAD_BEG TITLE_BEG "408 Request Timeout" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>408</b> Request Timeout" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::LENGTH_REQUIRED,
        HTML_BEG HEAD_BEG TITLE_BEG "411 Length Required" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>411</b> Length Required" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::PAYLOAD_TOO_LARGE,
        HTML_BEG HEAD_BEG TITLE_BEG "413 Payload Too Large" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>413</b> Payload Too Large" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::URI_TOO_LONG,
        HTML_BEG HEAD_BEG TITLE_BEG "414 URI Too Long" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>414</b> URI Too Long" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::UNSUPPORTED_MEDIA_TYPE,
        HTML_BEG HEAD_BEG TITLE_BEG "415 Unsupported Media Type" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>415</b> Unsupported Media Type" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::INTERNAL_SERVER_ERROR,
        HTML_BEG HEAD_BEG TITLE_BEG "500 Internal Server Error" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>500</b> Internal Server Error" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::NOT_IMPLEMENTED,
        HTML_BEG HEAD_BEG TITLE_BEG "501 Not Implemented" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>501</b> Not Implemented" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::BAD_GATEWAY,
        HTML_BEG HEAD_BEG TITLE_BEG "502 Bad Gateway" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>502</b> Bad Gateway" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::GATEWAY_TIMEOUT,
        HTML_BEG HEAD_BEG TITLE_BEG "504 Gateway Timeout" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>504</b> Gateway Timeout" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::HTTP_VERSION_NOT_SUPPORTED,
        HTML_BEG HEAD_BEG TITLE_BEG "505 HTTP Version Not Supported" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "505 HTTP Version Not Supported" H1_CENTER_END HR BODY_END HTML_END));
    _errorResponses.insert(std::make_pair(HTTP::UNKNOWN_ERROR,
        HTML_BEG HEAD_BEG TITLE_BEG "520 Unknown response status code" TITLE_END HEAD_END
        BODY_BEG H1_CENTER_BEG "<b>520</b> Unknown response status code" H1_CENTER_END HR BODY_END HTML_END));
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