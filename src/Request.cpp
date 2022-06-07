#include "Request.hpp"

namespace HTTP {

Request::Request()
    : _servBlock(NULL)
    , _location(NULL)
    , _client(NULL)
    , _flag_getline_bodySize(true)
    , _bodySize(0)
    , _parseFlags(PARSED_NONE)
    , _isAuthorized(false)
    , _storedHash(0) {
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
        _isAuthorized          = other._isAuthorized;
        _cookie               = other._cookie;
    }
    return *this;
}

ServerBlock *
Request::getServerBlock() const {
    return _servBlock;
}

void
Request::setServerBlock(ServerBlock *serverBlock) {
    _servBlock = serverBlock;
}

Location *
Request::getLocation(void) {
    return _location;
}

void
Request::setLocation(Location *location) {
    _location = location;
}

const Client *Request::getClient() const {
    return _client;
}

void Request::setClient(Client *client) {
    _client = client;
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

const std::map<uint32_t, Header> &
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

URI &
Request::getUriRef() {
    return _uri;
}

const std::string & 
Request::getRawUri() const {
    return _rawURI;
}

void
Request::setFlag(uint8_t flag) {
    _parseFlags |= flag;
}

void
Request::removeFlag(uint8_t flag) {
    _parseFlags &= ~flag;
}

uint32_t
Request::getStoredHash() const {
    return _storedHash;
}

void
Request::setStoredHash(uint32_t hash) {
    _storedHash = hash;
}

void
Request::clear() {
    _method = "";
    _protocol = "";
    _headers.clear();
    _body.clear();
    _bodySize = 0;
    _parseFlags = 0;
    _flag_getline_bodySize = true;
}

const std::string &
Request::getQueryString() const {
    return _uri._query;
}

const std::string &
Request::getResolvedPath() const {
    return _resolvedPath;
}

bool
Request::isAuthorized(void) const {
    return _isAuthorized;
}

void
Request::setAuthFlag(bool flag) {
    _isAuthorized = flag;
}

const std::string 
Request::getHeaderValue(uint32_t key) const {
    std::map<uint32_t, Header>::const_iterator it = _headers.find(key);
    if (it == _headers.end()) {
        return "";
    }
    return it->second.value;
}

int
Request::set(uint8_t flag) const {
    return _parseFlags & flag;
}

StatusCode
Request::parseLine(std::string line) {
    if (!set(PARSED_SL)) {
        return !line.empty() ? parseSL(line) : CONTINUE;
    } else if (!set(PARSED_HEADERS)) {
        return !line.empty() ? parseHeader(line) : checkHeaders();
    } else if (!set(PARSED_BODY)) {
        return (parseBody(line));
    } else {
        return PROCESSING;
    }
}

StatusCode
Request::parseSL(const std::string &line) {
  
    Log.debug("----------------------");
    Log.debug(line);
    
    size_t pos = 0;
    _method = getWord(line, ' ', pos);

    skipSpaces(line, pos);
    _rawURI = getWord(line, ' ', pos);
    Log.debug("PATH: " + _rawURI);
    
    skipSpaces(line, pos);
    _protocol = getWord(line, ' ', pos);

    skipSpaces(line, pos);
    if (line[pos]) {
        Log.debug("Forbidden symbols at the end of the SL");
        return BAD_REQUEST;
    }

    return checkSL();
}

StatusCode
Request::checkSL(void) {
    if (!isValidMethod(_method)) {
        Log.debug("Request::parseSL: Method " + _method + " is not implemented");
        return NOT_IMPLEMENTED;
    }

    // if (isValidPath(_rawURI)) {
    //     Log.debug("Invalid URI");
    //     return BAD_REQUEST;
    // }

    _uri.parse(_rawURI);

    if (!isValidProtocol(_protocol)) {
        return BAD_REQUEST;
    } else if (_major > 1) {
        return HTTP_VERSION_NOT_SUPPORTED;
    } else if (_major != 1 || _minor != 1) { // Supports only http/1.1 now
        return BAD_REQUEST;
    }
    setFlag(PARSED_SL);

    return CONTINUE;
}


// Need to move to another place so config and request could use the same array
bool
Request::isValidMethod(const std::string &method) {
    static const std::string validMethods[9] = {
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
    if (protocol.find("HTTP/") != 0) {
        return false;
    }
    if (!isdigit(protocol[5]) || protocol[6] != '.') {
        return false;
    }
    _major = protocol[5] - '0';
    if (!isdigit(protocol[7]) || protocol.length() != 8) {
        return false;
    }
    _minor = protocol[7] - '0';
    return true;
}

void
Request::resolvePath(void) {
    if (_location->getAliasRef().empty()) {
        _resolvedPath = _location->getRootRef();
        if (_uri._path[0] == '/' && _uri._path.length() > 1) {
            _resolvedPath += _uri._path.substr(1);
        }
    } else {
        _resolvedPath = _location->getAliasRef();
        if (_uri._path.length() > _location->getPathRef().length()) {
            _resolvedPath += _uri._path.substr(_location->getPathRef().length());
        }
    }
    Log.debug("Request::Resolved path:" + _resolvedPath);
}

StatusCode
Request::checkHeaders(void) {

    setFlag(PARSED_HEADERS);
    if (!isHeaderExist(HOST)) {
        Log.error("Host not found");
        return BAD_REQUEST;
    }

    // PUT or POST or PATCH
    if (_method[0] == 'P') {
        if (!isHeaderExist(TRANSFER_ENCODING) && !isHeaderExist(CONTENT_LENGTH)) {
            Log.error("Transfer-Encoding/Content-Length is missing in request");
            return LENGTH_REQUIRED;
        }
        return CONTINUE;
    }
    Log.debug("Request::ParsedHeaders");
    return PROCESSING;
}

StatusCode
Request::parseHeader(const std::string &line) {
    Header header;

    if (!header.parse(line)) {
        return BAD_REQUEST;
    }

    // dublicate header
    if (isHeaderExist(header.hash)) {
        Log.error("Dublicate header");
        return BAD_REQUEST;
    }

    header.handle(*this);
    _headers.insert(std::make_pair(header.hash, header));
    return CONTINUE;
}



StatusCode
Request::parseChunked(const std::string &line) {
    if (_flag_getline_bodySize) {
        return writeChunkedSize(line);
    }
    _flag_getline_bodySize = true;
    return (writeBody(line) == BAD_REQUEST ? BAD_REQUEST : CONTINUE);
}

StatusCode
Request::writeChunkedSize(const std::string &line) {
    if (line.empty() == true || line.find_first_not_of("0123456789ABCDEFabcdef") != line.npos) {
        // bad chunk length
        _flag_getline_bodySize = false;
        Log.error("Request: Incorrect format of the chunks size");
        return (BAD_REQUEST);
    }
    std::string chunk(line.c_str());
    _bodySize = strtoul(chunk.c_str(), NULL, 16);
    if (!_bodySize || _bodySize == ULONG_MAX) {
        if (!_bodySize && chunk[0] == '0') {
            setFlag(PARSED_BODY);
            return (PROCESSING);
        }
        Log.error("Request: Incorrect format of the chunks size");
        return (BAD_REQUEST);
    }
    _flag_getline_bodySize = false;
    return (CONTINUE);
}

StatusCode
Request::writeBody(const std::string &body) {
    if (body.length() > _bodySize) {
        Log.error("Request: the body length is too long");
        return (BAD_REQUEST);
    }
    _bodySize = 0;
    _body += body;
    return (PROCESSING);
}

bool
Request::isHeaderExist(const HeaderCode code) {
    return isHeaderExist(static_cast<uint32_t>(code));
}

bool
Request::isHeaderExist(const uint32_t code) {
    return (_headers.find(code) != _headers.end());
}

StatusCode
Request::parseBody(const std::string &line) {
    Log.debug("Request::parseBody");
    if (isHeaderExist(TRANSFER_ENCODING)) {
        return (parseChunked(line));
    } else if (isHeaderExist(TRANSFER_ENCODING)) {
        setFlag(PARSED_BODY);
        return writeBody(line);
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

const std::map<std::string, std::string> &
Request::getCookie(void) {
    return _cookie;
}

void
Request::setCookie(std::map<std::string, std::string> cookie) {
    _cookie = cookie;
}

} // namespace HTTP
