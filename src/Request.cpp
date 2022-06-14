#include "Request.hpp"
#include "Client.hpp"
#include "Server.hpp"

namespace HTTP {

Request::Request()
    : _servBlock(NULL)
    , _location(NULL)
    , _client(NULL)
    , _isChuckSize(false)
    , _chunkSize(0)
    , _bodySize(0)
    , _parseFlags(PARSED_NONE)
    , _isAuthorized(false)
    , _storedHash(0)
    , _isFormed(false) {
}

Request::~Request() { }

Request::Request(const Request &other) {
    *this = other;
}

Request &
Request::operator=(const Request &other) {
    if (this != &other) {
        _method       = other._method;
        _uri          = other._uri;
        _protocol     = other._protocol;
        _resolvedPath = other._resolvedPath;
        _headers      = other._headers;
        _status       = other._status;
        _servBlock    = other._servBlock;
        _location     = other._location;
        _isChuckSize  = other._isChuckSize;
        _bodySize     = other._bodySize;
        _body         = other._body;
        _parseFlags   = other._parseFlags;
        _isAuthorized = other._isAuthorized;
        _cookie       = other._cookie;
        _chunkSize    = other._chunkSize;
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

const Client *
Request::getClient() const {
    return _client;
}

void
Request::setClient(Client *client) {
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

const std::map<uint32_t, RequestHeader> &
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

URI &
Request::getHostRef() {
    return _host;
}

const std::string &
Request::getRawUri() const {
    return _rawURI;
}

bool
Request::isFormed(void) const {
    return _isFormed;
}

void
Request::isFormed(bool formed) {
    _isFormed = formed;
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
    _method   = "";
    _rawURI   = "";
    _protocol = "";
    _uri.clear();
    _headers.clear();
    _body.clear();
    _resolvedPath = "";
    _bodySize     = 0;
    _parseFlags   = 0;
    _chunkSize    = 0;
    _isChuckSize  = false;
    _isFormed = false;

    _headers.clear();
    _status = OK;
    _minor  = 0;
    _major  = 0;
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
Request::authNeeded(void) {
    return getLocation()->getAuthRef().isSet();
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
    // return _headers[key];
    std::map<uint32_t, RequestHeader>::const_iterator it = _headers.find(key);
    if (it == _headers.end()) {
        return "";
    }
    return it->second.value;
}

int
Request::set(uint8_t flag) const {
    return _parseFlags & flag;
}

void
Request::parseLine(std::string &line) {
    if (!set(PARSED_SL)) {
        rtrim(line, "\r\n");
        setStatus(!line.empty() ? parseSL(line) : CONTINUE);
    } else if (!set(PARSED_HEADERS)) {
        rtrim(line, "\r\n");
        setStatus(!line.empty() ? parseHeader(line) : checkHeaders());
    } else if (!set(PARSED_BODY)) {
        setStatus(parseBody(line));
    } else {
        setStatus(PROCESSING);
    }

    if (getStatus() != CONTINUE) {
        if (!_servBlock) {
            setServerBlock(g_server->matchServerBlock(getClient()->getServerPort(), "", _host._host));
        }
        if (!_location) {
            setLocation(getServerBlock()->matchLocation(_uri._path));
        }
        isFormed(true);
    }

}

StatusCode
Request::parseSL(const std::string &line) {

    Log.debug("----------------------");
    Log.debug(line);

    size_t pos = 0;
    _method    = getWord(line, " ", pos);

    skipSpaces(line, pos);
    _rawURI = getWord(line, " ", pos);
    _uri.parse(_rawURI);

    skipSpaces(line, pos);
    _protocol = getWord(line, " ", pos);

    skipSpaces(line, pos);
    if (line[pos]) {
        Log.debug("Forbidden symbols at the end of the SL: ");
        return BAD_REQUEST;
    }

    return checkSL();
}

StatusCode
Request::checkSL(void) {
    if (!isValidMethod(_method)) {
        Log.debug("Request::parseSL: Method " + _method + " is not implemented");
        return BAD_REQUEST;
    }

    // if (isValidPath(_rawURI)) {
    //     Log.debug("Invalid URI");
    //     return BAD_REQUEST;
    // }

    if (!isValidProtocol(_protocol)) {
        Log.debug("Request::checkSL: protocol " + _protocol + " is not valid");
        return BAD_REQUEST;
    } else if (_major > 1) {
        Log.debug("Request::checkSL: protocol " + _protocol + " is not supported");
        return HTTP_VERSION_NOT_SUPPORTED;
    } else if (_major != 1 || _minor != 1) {
        Log.debug("Request::checkSL: protocol " + _method + " is not implemented");
        return BAD_REQUEST;
    }

    if (!_servBlock) {
        setServerBlock(getClient()->matchServerBlock(_uri._host));
    }
    if (!_location) {
        setLocation(getServerBlock()->matchLocation(_uri._path));
    }
    resolvePath();

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
    Log.debug("Request::Resolved path: " + _resolvedPath);
}

StatusCode
Request::checkHeaders(void) {

    setFlag(PARSED_HEADERS);
    // if (!isHeaderExist(HOST)) {
    //     Log.error("Request:: Host not found");
    //     return BAD_REQUEST;
    // }

    // Call each header handler
    std::map<uint32_t, RequestHeader>::iterator it;
    for (it = _headers.begin(); it != _headers.end(); it++) {
        StatusCode status = it->second.handle(*this);
        if (status != CONTINUE) {
            return status;
        }
    }

    std::vector<std::string> &allowed = getLocation()->getAllowedMethodsRef();
    if (std::find(allowed.begin(), allowed.end(), _method) == allowed.end()) {
        Log.debug("Request:: Method " + _method + " is not allowed");
        return METHOD_NOT_ALLOWED;
    }

    if (!isHeaderExist(TRANSFER_ENCODING) && !isHeaderExist(CONTENT_LENGTH)) {
        // PUT or POST or PATCH
        if (_method[0] == 'P') {
            Log.error("Request::Transfer-Encoding/Content-Length is missing in request");
            return LENGTH_REQUIRED;
        } else {
            Log.debug("Request::ParsedHeaders::Processing");
            return PROCESSING;
        }
    }
    Log.debug("Request::ParsedHeaders::Continue");
    return CONTINUE;
}

StatusCode
Request::parseHeader(const std::string &line) {
    RequestHeader header;

    if (!header.parse(line)) {
        Log.debug("ParseHeader:: Invalid header " + line);
        return BAD_REQUEST;
    }

    // dublicate header
    if (isHeaderExist(header.hash)) {
        Log.debug("ParseHeader:: Dublicated header");
        return BAD_REQUEST;
    }

    // header.handle(*this);
    _headers.insert(std::make_pair(header.hash, header));
    return CONTINUE;
}

StatusCode
Request::writeChuck(const std::string &chunk) {

    if (_chunkSize >= chunk.length()) {
        _body += chunk;
        _chunkSize -= chunk.length();

    } else {
        if (chunk[_chunkSize] != '\r' || 
            chunk[_chunkSize + 1] != '\n') {
            Log.error("Request:: Invalid chuck length");
            return BAD_REQUEST;
        }
        _body += chunk.substr(0, chunk.length() - 2);
        _chunkSize = 0;
        _isChuckSize = true;
    }

    return CONTINUE;
}

StatusCode
Request::parseChunk(const std::string &line) {    
    return (_isChuckSize ? writeChunkSize(line) : writeChuck(line));
}

StatusCode
Request::writeChunkSize(const std::string &line) {

    if (line.empty()) {
        Log.error("Request:: Chunk size is empty");
        return BAD_REQUEST;
    }

    char *end = NULL;
    _chunkSize = strtoul(line.c_str(), &end, 16);

    if (!end || end[0] != '\r' || end[1] != '\n') {
        Log.error("Request:: Chunks size is invalid: " + line);
        return BAD_REQUEST;

    } else if (_chunkSize == ULONG_MAX) {
        Log.error("Request:: Chunk size is ULONG_MAX: " + line);
        return BAD_REQUEST;

    } else if (_chunkSize == 0 && line[0] != '0') {
        Log.error("Request:: Chunk size parsing failed: " + line);
        return BAD_REQUEST;

    } else if (_chunkSize == 0) {
        setFlag(PARSED_BODY);
        return PROCESSING;
    }

    _isChuckSize = false;
    return CONTINUE;
}

StatusCode
Request::writeBody(const std::string &body) {
    Log.debug("Request::writeBody " + body);

    if (body.length() > _bodySize) {
        Log.error("Request: the body length is too long");
        return BAD_REQUEST;
    }
    _body = body;
    setFlag(PARSED_BODY);
    return PROCESSING;
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
    Log.debug("Request::parseBody " + line);
    if (isHeaderExist(TRANSFER_ENCODING)) {
        Log.debug("Request::parseChunk");
        return parseChunk(line);
    } else if (isHeaderExist(CONTENT_LENGTH)) {
        Log.debug("Request::writeBody");
        return writeBody(line);
    }
    return PROCESSING;
}

// for chunked
bool
Request::getBodySizeFlag() {
    return (_isChuckSize);
}

void
Request::setBodySizeFlag(bool isSize) {
    _isChuckSize = isSize;
}

size_t
Request::getBodySize() {
    return (_bodySize);
}
void
Request::setBodySize(size_t size) {
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
