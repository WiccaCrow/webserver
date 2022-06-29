#include "AResponse.hpp"
#include "CGI.hpp"
#include "Client.hpp"

namespace HTTP {

AResponse::AResponse() 
    : _client(NULL)
    , _location(NULL)
    , _bodyLength(0)
    , _formed(false) {}

AResponse::AResponse(Request *req) 
    : _client(NULL)
    , _location(NULL)
    , _bodyLength(0)
    , _formed(false)
    , _status(OK)
{
    _mandatory.push_back(ResponseHeader(DATE));
    _mandatory.push_back(ResponseHeader(SERVER));
    _mandatory.push_back(ResponseHeader(KEEP_ALIVE));
    _mandatory.push_back(ResponseHeader(CONNECTION));
    _mandatory.push_back(ResponseHeader(CONTENT_TYPE));
    _mandatory.push_back(ResponseHeader(CONTENT_LENGTH));
    _mandatory.push_back(ResponseHeader(ACCEPT_RANGES));
}

AResponse::AResponse(const AResponse &other) {
    *this = other;
}

AResponse &AResponse::operator=(const AResponse &other) {
    if (this != &other) {
        _client = other._client;
        _body = other._body;
        _bodyLength = other._bodyLength;
        _formed = other._formed;
        _status = other._status;
        _headers = other._headers;
        _mandatory = other._mandatory;
    }
    return *this;
}

AResponse::~AResponse() {}

void
AResponse::shouldBeClosedIf(void) {

    static const size_t size = 5;
    static const StatusCode failedStatuses[size] = {
        BAD_REQUEST,
        REQUEST_TIMEOUT,
        INTERNAL_SERVER_ERROR,
        PAYLOAD_TOO_LARGE,
        UNAUTHORIZED
    };

    for (size_t i = 0; i < size; i++) {
        if (getStatus() == failedStatuses[i]) {
            getClient()->shouldBeClosed(true);
            addHeader(CONNECTION, "close");
            break ;
        }
    }
}

void
AResponse::handle(void) {

    if (getStatus() < BAD_REQUEST) {
        perform();
    }
    
    if (getStatus() >= BAD_REQUEST) {
        assembleErrorBody();
        shouldBeClosedIf();
    }

    if (!formed()) {
        assembleHead();
        formed(true);
    }
}

void
AResponse::assembleErrorBody(void) {

    if (getLocation() != NULL) {

        std::map<int, std::string> &pages = getLocation()->getErrorPagesRef();
        std::map<int, std::string>::iterator it = pages.find(getStatus());

        if (it != pages.end()) {
            setBody(readFile(it->second));
            if (!getBody().empty()) {
                return ;
            }
        }
    }

    if (!errorResponses.has(getStatus())) {
        Log.error() << "Unknown response code: " << static_cast<int>(getStatus()) << Log.endl;
        setStatus(UNKNOWN_ERROR);   
    }
    setBody(errorResponses[getStatus()]);
}

void
AResponse::assembleHead(void) {

    _head.reserve(512);
    _head = statusLines[getStatus()];

    for (iterator it = _headers.begin(); it != _headers.end(); ++it) {
        if (it->value.empty()) {
            it->handle(*this);
        }
        if (!it->value.empty()) {
            _head += headerNames[it->hash] + ": " + it->value + "\r\n";
        }
    }

    // if (_cgi != NULL) {
    //     const_iter it = _cgi->getExtraHeaders().begin();
    //     for ( ; it != _cgi->getExtraHeaders().end(); ++it) {
    //         _head += it->key + ": " + it->value + "\r\n";
    //     }
    // }

    _head += "\r\n";
}

ResponseHeader *
AResponse::getHeader(uint32_t hash) {
    iterator it = std::find(_headers.begin(), _headers.end(), ResponseHeader(hash));

    if (it == _headers.end()) {
        return NULL;
    } else {
        return &(*it);
    }
}

void
AResponse::addHeader(uint32_t hash, const std::string &value) {
    ResponseHeader *ptr = getHeader(hash);
    if (ptr == NULL) {
        _headers.push_back(ResponseHeader(hash, value));
    } else {
        ptr->value = value;
    }
}

void
AResponse::addHeader(uint32_t hash) {
    addHeader(hash, "");
}

// Rewrite
// void
// AResponse::makeChunk() {
//     _res = "";
//     if (!getHeader(TRANSFER_ENCODING) ) {
//         return ;
//     }
//     if (_resourceFileStream.eof()) {
//         _resourceFileStream.close();
//         _resourceFileStream.clear();
//         return ;
//     }
//     char buffer[CHUNK_SIZE] = {0};
//     _resourceFileStream.read(buffer, sizeof(buffer) - 1);
//     if (_resourceFileStream.fail() && !_resourceFileStream.eof()) {
//         setStatus(INTERNAL_SERVER_ERROR);
//         _client->shouldBeClosed(true);
//         return ;
//     } else {
//         _res.assign(buffer, _resourceFileStream.gcount());
//         _res = itohs(_resourceFileStream.gcount()) + "\r\n" + _res + "\r\n";
//     }
//     if (_resourceFileStream.eof()) {
//         _res += "0\r\n\r\n";
//     }
// }

const std::string &
AResponse::getBody() const {
    return _body;
}

const std::string &
AResponse::getHead() const {
    return _head;
}

void
AResponse::setBody(const std::string &body) {
    _body = body;
    setBodyLength(_body.length());
}

size_t
AResponse::getBodyLength(void) const {
    return _bodyLength;
}

void
AResponse::setBodyLength(size_t len) {
    _bodyLength = len;
}

StatusCode
AResponse::getStatus() {
    return _status;
}

void
AResponse::setStatus(StatusCode status) {
    _status = status;
}

bool
AResponse::formed(void) const {
    return _formed;
}

void
AResponse::formed(bool formed) {
    _formed = formed;
}

} // namespace HTTP
