#include "Request.hpp"

namespace HTTP {

Request::Request()
    : _servBlock(NULL)
    , _location(NULL)
    , _flag_getline_bodySize(true)
    , _bodySize(0)
    , _parseFlags(PARSED_NONE) {
}

Request::~Request() { }

Request::Request(const Request &other) {
    *this = other;
}

Request &
Request::operator=(const Request &other) {
    if (this != &other) {
        _method                = other._method;
        _uri                   = other._uri;
        _protocol              = other._protocol;
        _resolvedPath          = other._resolvedPath;
        _headers               = other._headers;
        _status                = other._status;
        _servBlock             = other._servBlock;
        _location              = other._location;
        _flag_getline_bodySize = other._flag_getline_bodySize;
        _bodySize              = other._bodySize;
        _body                  = other._body;
        _parseFlags            = other._parseFlags;
    }
    return *this;
}

const ServerBlock *
Request::getServerBlock() const {
    return _servBlock;
}

void
Request::setServerBlock(ServerBlock *serverBlock) {
    _servBlock = serverBlock;
}

const std::string &
Request::getMethod() const {
    return _method;
}

const std::string &
Request::getPath() const {
    return _uri._path;
}

const std::string &
Request::getProtocol() const {
    return _protocol;
}

const std::map<HeaderCode, Header> &
Request::getHeaders() const {
    return _headers;
}

const std::string &
Request::getBody() const {
    return _body;
}

const uint8_t &
Request::getFlags() const {
    return _parseFlags;
}

const HTTP::StatusCode &
Request::getStatus() const {
    return _status;
}

Location *
Request::getLocationPtr() {
    return _location;
}

URI &
Request::getUriRef() {
    return _uri;
}

const std::string & 
Request::getRawUri() const {
    return _rawURI;
}

// bool
// Request::empty() {
//     return (_method.empty() && _uri.empty() && _protocol.empty() && _headers.empty());
// }

void
Request::setFlag(uint8_t flag) {
    _parseFlags |= flag;
}

void
Request::removeFlag(uint8_t flag) {
    _parseFlags &= ~flag;
}

void
Request::clear() {
    _method = "";
    // _uri.clear();
    _protocol = "";
    _headers.clear();
    _flag_getline_bodySize = true;
    _bodySize              = 0;
    _body.clear();
    _parseFlags = 0;

    // _method.clear();
    // _uri.clear();
    // _protocol.clear();
    // _headers.clear();
    // _flag_getline_bodySize = true;
    // _bodySize = 0;
    // _body.clear();
    // _parseFlags = 0;
}

const std::string &
Request::getQueryString() const {
    return _uri._query;
}

const std::string &
Request::getResolvedPath() const {
    return _resolvedPath;
}

const std::string 
Request::getHeaderValue(HeaderCode key) const {
    std::map<HeaderCode, Header>::const_iterator it = _headers.find(key);
    if (it == _headers.end()) {
        return "";
    }
    return it->second.value;
}

StatusCode
Request::parseLine(std::string line) {
    if (!(getFlags() & PARSED_SL)) {
        if ((_status = parseStartLine(line)) != HTTP::CONTINUE) {
            Log.error("Request::parseLine, parsing SL");
            // return HTTP::Response(_status);
            return _status;
        }
    } else if (!(getFlags() & PARSED_HEADERS)) {
        if ((_status = parseHeader(line)) != HTTP::CONTINUE) {
            Log.error("Request::parseLine, parsing Headers");
            // return HTTP::Response(_status);
            return _status;
        }
    } else if (!(getFlags() & PARSED_BODY)) {
        if ((_status = parseBody(line)) != HTTP::CONTINUE) {
            Log.error("Request::parseLine, parsing Body");
            return _status;
        }
    } else {
        return (_status = PROCESSING);
    }
    return CONTINUE;
}

StatusCode
Request::parseStartLine(const std::string &line) {
    if (line.empty()) {
        return CONTINUE;
    } // ?
    Log.debug("###################################################################");
    Log.debug(line);
    size_t pos = 0;
    _method    = getWord(line, ' ', pos);
    Log.debug("METHOD: " + _method);
    if (!isValidMethod(_method)) {
        Log.debug("Request::parseStartLine: Method is not implemented");
        return NOT_IMPLEMENTED;
    }

    skipSpaces(line, pos);
    _rawURI = getWord(line, ' ', pos);
    Log.debug("PATH: " + _rawURI);
    _uri.parse(_rawURI);
    // if (isValidPath(_rawURI)) {
    //     Log.debug("Invalid URI");
    //     return BAD_REQUEST;
    // }

    skipSpaces(line, pos);
    _protocol = getWord(line, ' ', pos);

    skipSpaces(line, pos);
    if (line[pos]) {
        Log.debug("Forbidden symbols at the end of the line");
        return BAD_REQUEST;
    }
    if (!isValidProtocol(_protocol)) {
        Log.debug("Protocol is not supported or invalid");
        // Or 505, need to improve
        return BAD_REQUEST;
    }

    setFlag(PARSED_SL);
    return CONTINUE;
}

bool
Request::isValidMethod(const std::string &method) {
    std::string validMethods[9] = {
        "GET", "DELETE", "POST",
        "PUT", "HEAD", "CONNECT",
        "OPTIONS", "TRACE", "PATCH"
    };
    for (int i = 0; i < 9; ++i) {
        if (validMethods[i] == method)
            return true;
    }
    return false;
}

// Should be moved later
bool
Request::isValidProtocol(const std::string &protocol) {
    return (protocol == "HTTP/1.1");
}

StatusCode
Request::parseHeader(std::string line) {
    if (line == "") {
        setFlag(PARSED_HEADERS);
        Log.debug("Request::parseHeader: Headers are parsed");

        _location = _servBlock->matchLocation(_uri._path);
        if (_location->getAliasRef() == "") {
            _resolvedPath = _location->getRootRef();
            if (_uri._path[0] == '/' && _uri._path.length() > 1) {
                _resolvedPath += _uri._path.substr(1);
            }
        } else {
            // Alias used, watch carefully about / case
            Log.info("alias: " + _location->getAliasRef());
            Log.info("uri.path: " + _uri._path);
            _resolvedPath = _location->getAliasRef();
            if (_uri._path.length() > _location->getPathRef().length()) {
                Log.info("append: " + _uri._path.substr(_location->getPathRef().length()));
                _resolvedPath += _uri._path.substr(_location->getPathRef().length());
            }
        }
        Log.debug("PHYSICAL_PATH:" + _resolvedPath);

        // transfer-encoding && content-length not find
        if (_headers.find(TRANSFER_ENCODING) == _headers.end() && _headers.find(CONTENT_LENGTH) == _headers.end()) {
            return PROCESSING;
        }
        return CONTINUE;
    }
    // Log.debug(line); // Print headers
    size_t sepPos = line.find(':');
    if (sepPos == std::string::npos) {
        return BAD_REQUEST;
    } 

    toLowerCase(line);

    Header header;
    header.key = line.substr(0, sepPos);
    header.value = line.substr(sepPos + 1);
    
    trim(header.value, " \t\r\n");

    header.hash = static_cast<HeaderCode>(crc(header.key.c_str(), header.key.length()));

    if (header.handle(*this) == BAD_REQUEST) {
        // Not all unsupported headers should lead to bad request
        Log.debug("Request::parseHeader: Maybe header is not supported");
        Log.debug(header.key + "|" + to_string((unsigned long)header.hash));
        return BAD_REQUEST;
    }
    
    // dublicate header
    if (_headers.find(header.hash) != _headers.end()) {
        return BAD_REQUEST;
    }

    // Copying here need to replace
    _headers.insert(std::make_pair(header.hash, header));

    return CONTINUE;
}

StatusCode
Request::parseChunked(const std::string &line) {
    if (_flag_getline_bodySize) {
        if (line.empty() == true || line.find_first_not_of("0123456789ABCDEFabcdef") != line.npos) {
            // bad chunk length
            _flag_getline_bodySize = false;
            return (BAD_REQUEST);
        }
        std::string chunk(line.c_str());
        if ((_bodySize = strtoul(chunk.c_str(), NULL, 16)) == 0) {
            if (chunk[0] == '0') {
                setFlag(PARSED_BODY);
                return (PROCESSING);
            }
            return (BAD_REQUEST);
        }
        _flag_getline_bodySize = false;
        return (CONTINUE);
    }

    _flag_getline_bodySize = true;
    if (line.length() > _bodySize) {
        // bad chunk body
        return (BAD_REQUEST);
    }
    _bodySize = 0;
    _body += line;
    return (CONTINUE);
}

StatusCode
Request::parseBody(const std::string &line) {
    Log.debug("Request::parseBody");
    if (_headers.find(TRANSFER_ENCODING) != _headers.end()) {
        return (parseChunked(line));
    } else if (_headers.find(CONTENT_LENGTH) != _headers.end()) {
        setFlag(PARSED_BODY);
        parseChunked(line);
        return PROCESSING;
    }
    return PROCESSING;
}

// for chunked
bool
Request::getBodySizeFlag() {
    return (_flag_getline_bodySize);
}

void
Request::setBodySizeFlag(bool isSize) {
    _flag_getline_bodySize = isSize;
}

unsigned long
Request::getBodySize() {
    return (_bodySize);
}
void
Request::setBodySize(unsigned long size) {
    _bodySize = size;
}

void
Request::setStatus(const HTTP::StatusCode &status) {
    _status = status;
}

} // namespace HTTP
