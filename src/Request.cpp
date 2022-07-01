#include "Request.hpp"
#include "Client.hpp"
#include "Server.hpp"

namespace HTTP {

ARequest::ARequest(void)
    : _major(0)
    , _minor(0) 
    , _isChunkSize(false)
    , _bodySize(0)
    , _chunkSize(0)
    , _parseFlags(PARSED_NONE)
    , _formed(false)
    , _status(OK) {}

ARequest::~ARequest(void) {}
ARequest::ARequest(const ARequest &other) {
    *this = other;
}

ARequest &
ARequest::operator=(const ARequest &other) {
    if (this != &other) {
        _body         = other._body;
        _head         = other._head;
        _bodySize     = other._bodySize;
        _isChunkSize  = other._isChunkSize;
        _parseFlags   = other._parseFlags;
        _chunkSize    = other._chunkSize;
        _formed       = other._formed;
        _chunked      = other._chunked;
        _status       = other._status;
        _minor        = other._minor;
        _major        = other._major;
        _protocol     = other._protocol;

    }
    return *this;
}

const std::string &
ARequest::getBody(void) const {
    return _body;
}

const std::string &
ARequest::getHead(void) const {
    return _head;
}

const std::string &
ARequest::getProtocol(void) const {
    return _protocol;
}

int
ARequest::getMajor(void) {
    return _major;
}
int
ARequest::getMinor(void) {
    return _minor;
}
void
ARequest::setBody(const std::string &body) {
    _body = body;
}

void
ARequest::setHead(const std::string &head) {
    _head = head;
}


std::size_t
ARequest::getFlags(void) const {
    return _parseFlags;
}

StatusCode
ARequest::getStatus() const {
    return _status;
}

bool
ARequest::formed(void) const {
    return _formed;
}

void
ARequest::formed(bool formed) {
    _formed = formed;
}

void
ARequest::setProtocol(const std::string &protocol) {
    _protocol = protocol;
}

void
ARequest::setMajor(int major) {
    _major = major;
}

void
ARequest::setMinor(int minor) {
    _minor = minor;
}

void
ARequest::setFlag(std::size_t flag) {
    _parseFlags |= flag;
}

void
ARequest::chunked(bool flag) {
    _chunked = flag;
}

bool
ARequest::chunked(void) const {
    return _chunked;
}

bool
ARequest::flagSet(std::size_t flag) const {
    return _parseFlags & flag;
}

// for chunked
bool
ARequest::isChunkSize(void) const {
    return _isChunkSize;
}

void
ARequest::isChunkSize(bool flag) {
    _isChunkSize = flag;
}

std::size_t
ARequest::getBodySize(void) const {
    return _bodySize;
}

void
ARequest::setBodySize(std::size_t size) {
    _bodySize = size;
}

void
ARequest::setStatus(StatusCode status) {
    _status = status;
}

StatusCode
ARequest::writeChuck(const std::string &chunk) {

    if (_chunkSize >= chunk.length()) {
        _body += chunk;
        _chunkSize -= chunk.length();

    } else {
        if (chunk[_chunkSize] != '\r' || 
            chunk[_chunkSize + 1] != '\n') {
            Log.error() << "ARequest:: Invalid chuck length" << Log.endl;
            return BAD_REQUEST;
        }
        _body += chunk.substr(0, chunk.length() - 2);
        _chunkSize = 0;
        isChunkSize(true);
    }

    return CONTINUE;
}

StatusCode
ARequest::parseChunk(const std::string &line) {    
    return (isChunkSize() ? writeChunkSize(line) : writeChuck(line));
}

StatusCode
ARequest::writeChunkSize(const std::string &line) {

    if (line.empty()) {
        Log.error() << "Request:: Chunk size is empty" << Log.endl;
        return BAD_REQUEST;
    }

    char *end = NULL;
    _chunkSize = strtoul(line.c_str(), &end, 16);

    if (!end || end[0] != '\r' || end[1] != '\n') {
        Log.error() << "Request:: Chunks size is invalid: " << line << Log.endl;
        return BAD_REQUEST;

    } else if (_chunkSize == ULONG_MAX) {
        Log.error() << "Request:: Chunk size is ULONG_MAX: " << line << Log.endl;
        return BAD_REQUEST;

    } else if (_chunkSize == 0 && line[0] != '0') {
        Log.error() << "Request:: Chunk size parsing failed: " << line << Log.endl;
        return BAD_REQUEST;

    } else if (_chunkSize == 0) {
        setFlag(PARSED_BODY);
        return PROCESSING;
    }

    isChunkSize(false);
    return CONTINUE;
}

Request::Request(void) 
    : ARequest()
    , _servBlock(NULL)
    , _location(NULL)
    , _client(NULL)
    , _storedHash(0)
    , _authorized(false) {}

Request::Request(Client *client)
    : ARequest()
    , _servBlock(NULL)
    , _location(NULL)
    , _client(client)
    , _storedHash(0) 
    , _authorized(false) {}

Request::~Request() {}

Request::Request(const Request &other) {
    *this = other;
}

Request &
Request::operator=(const Request &other) {
    if (this != &other) {
        _method       = other._method;
        _uri          = other._uri;
        _rawURI       = other._rawURI;
        _resolvedPath = other._resolvedPath;
        _servBlock    = other._servBlock;
        _location     = other._location;
        _authorized   = other._authorized;
        _cookie       = other._cookie;
        _client       = other._client;
        _host         = other._host;
        _storedHash   = other._storedHash;
       
        headers      = other.headers;
    }
    return *this;
}

const std::string &
Request::getMethod() const {
    return _method;
}

const std::string &
Request::getPath() const {
    return _uri._path;
}



ServerBlock *
Request::getServerBlock(void) const {
    return _servBlock;
}

void
Request::setServerBlock(ServerBlock *servBlock) {
    _servBlock = servBlock;
}

Location *
Request::getLocation(void) const {
    return _location;
}

void
Request::setLocation(Location *location) {
    _location = location;
}

Client *
Request::getClient(void) {
    return _client;
}

void
Request::setClient(Client *client) {
    _client = client;
}


URI &
Request::getUriRef() {
    return _uri;
}

URI &
Request::getHostRef() {
    return _host;
}

URI &
Request::getReferrerRef() {
    return _host;
}


const std::string &
Request::getRawUri() const {
    return _rawURI;
}

uint32_t
Request::getStoredHash() const {
    return _storedHash;
}

RangeList &
Request::getRangeList(void) {
    return _ranges;
}

void
Request::setStoredHash(uint32_t hash) {
    _storedHash = hash;
}



const std::string &
Request::getResolvedPath() const {
    return _resolvedPath;
}

void
Request::setResolvedPath(const std::string &path) {
    _resolvedPath = path;
}

bool
Request::authorized(void) const {
    return !getLocation()->getAuthRef().isSet() || _authorized;
}

void
Request::authorized(bool flag) {
    _authorized = flag;
}


// Maybe should be moved back to Client?
HTTP::ServerBlock *
matchServerBlock(const std::string &host, std::size_t port, const std::string &ip) {
    typedef std::list<HTTP::ServerBlock>::iterator iter_l;
    typedef std::list<HTTP::ServerBlock> bslist;

    bool searchByName = !isValidIpv4(host) ? true : false;

    bslist &blocks = g_server->operator[](port);
    iter_l found = blocks.end();
    for (iter_l block = blocks.begin(); block != blocks.end(); ++block) {
        if (block->hasAddr(ip)) {
            if (found == blocks.end()) {
                found = block;
                if (!searchByName) {
                    break ;
                }
            }
            if (searchByName && block->hasName(host)) {
                found = block;
                break ;
            }
        }
    }
    Log.debug() << "Client:: servBlock " << found->getBlockName() << " for " << host << ":" << port << Log.endl;
    return &(*found);
}

bool
Request::parseLine(std::string &line) {

    if (!flagSet(PARSED_SL)) {
        rtrim(line, CRLF);
        setStatus(!line.empty() ? parseSL(line) : CONTINUE);
    } else if (!flagSet(PARSED_HEADERS)) {
        rtrim(line, CRLF);
        setStatus(!line.empty() ? parseHeader(line) : checkHeaders());
    } else if (!flagSet(PARSED_BODY)) {
        setStatus(parseBody(line));
    } else {
        setStatus(PROCESSING);
    }

    if (getStatus() != CONTINUE) {
        if (!_servBlock) {
            Log.debug() << "Serverblock is not set after parsing. Default matching" << Log.endl;
            setServerBlock(matchServerBlock(_host._host, getClient()->getServerSock()->getPort(), getClient()->getServerSock()->getAddr()));
        }
        if (!_location) {
            Log.debug() << "Location is not set after parsing. Default matching" << Log.endl;
            setLocation(getServerBlock()->matchLocation(_uri._path));
            resolvePath();
        }
        formed(true);

        if (getStatus() == PROCESSING) {
            setStatus(OK);
        }
    }
    return formed();
}

StatusCode
Request::parseSL(const std::string &line) {

    Log.debug() << line << Log.endl;

    std::size_t pos = 0;
    _method    = getWord(line, " ", pos);

    skipSpaces(line, pos);
    _rawURI = getWord(line, " ", pos);
    _uri.parse(_rawURI);

    skipSpaces(line, pos);
    setProtocol(getWord(line, " ", pos));

    skipSpaces(line, pos);
    if (line[pos]) {
        Log.debug() << "Forbidden symbols at the end of the SL: " << Log.endl;
        return BAD_REQUEST;
    }

    return checkSL();
}

StatusCode
Request::checkSL(void) {
    if (!isValidMethod(_method)) {
        Log.debug() << "Request::parseSL: Method " << _method << " is not implemented" << Log.endl;
        return BAD_REQUEST;
    }

    // if (isValidPath(_rawURI)) {
    //     Log.debug() << "Invalid URI" << Log.endl;
    //     return BAD_REQUEST;
    // }

    if (!isValidProtocol(getProtocol())) {
        Log.debug() << "Request::checkSL: protocol " << getProtocol() << " is not valid" << Log.endl;
        return BAD_REQUEST;
    } 
    setMajor(getProtocol()[5] - '0');
    setMinor(getProtocol()[7] - '0');
    if (getMajor() > 1) {
        Log.debug() << "Request::checkSL: protocol " << getProtocol() << " is not supported" << Log.endl;
        return HTTP_VERSION_NOT_SUPPORTED;
    } else if (getMajor() != 1 || getMinor() != 1) {
        Log.debug() << "Request::checkSL: protocol " << _method << " is not implemented" << Log.endl;
        return BAD_REQUEST;
    }

    setFlag(PARSED_SL);
    return CONTINUE;
}

void
Request::resolvePath(void) {
    if (_location->getAliasRef().empty()) {
        setResolvedPath(_location->getRootRef());
        if (_uri._path[0] == '/' && _uri._path.length() > 1) {
            _resolvedPath += _uri._path.substr(1);
        }
    } else {
        setResolvedPath(_location->getAliasRef());
        if (_uri._path.length() > _location->getPathRef().length()) {
            _resolvedPath += _uri._path.substr(_location->getPathRef().length());
        }
    }
    setResolvedPath(URI::URLdecode(_resolvedPath));
    Log.debug() << "Request::resolvePath: " << _resolvedPath << Log.endl;
}

StatusCode
Request::checkHeaders(void) {

    setFlag(PARSED_HEADERS);
    
    // We should not check header host, but host entity itself 
    // if (!isHeaderExist(HOST)) {
    //     Log.error() << "Request:: Host not found" << Log.endl;
    //     return BAD_REQUEST;
    // }

    if (!_servBlock) {
        setServerBlock(matchServerBlock(_uri._host, getClient()->getServerSock()->getPort(), getClient()->getServerSock()->getAddr()));
    }
    if (!_location) {
        setLocation(getServerBlock()->matchLocation(_uri._path));
        resolvePath();
    }

    // Call each header handler
    Headers<RequestHeader>::iterator it;
    for (it = headers.begin(); it != headers.end(); ++it) {
        StatusCode status = it->second.handle(*this);
        if (status != CONTINUE) {
            return status;
        }
    }

    std::vector<std::string> &allowed = getLocation()->getAllowedMethodsRef();
    if (std::find(allowed.begin(), allowed.end(), _method) == allowed.end()) {
        Log.debug() << "Request:: Method " << _method << " is not allowed" << Log.endl;
        return METHOD_NOT_ALLOWED;
    }

    if (!headers.has(TRANSFER_ENCODING) && !headers.has(CONTENT_LENGTH)) {
        // PUT or POST or PATCH
        if (_method[0] == 'P') {
            Log.error() << "Request::Transfer-Encoding/Content-Length is missing in request" << Log.endl;
            return LENGTH_REQUIRED;
        } else {
            Log.debug() << "Request::ParsedHeaders::Processing" << Log.endl;
            return PROCESSING;
        }
    }
    Log.debug() << "Request::ParsedHeaders::Continue" << Log.endl;
    return CONTINUE;
}

StatusCode
Request::parseHeader(const std::string &line) {
    RequestHeader header;

    if (!header.parse(line)) {
        Log.debug() << "ARequest:: Invalid header " << line << Log.endl;
        return BAD_REQUEST;
    }

    // dublicate header
    if (headers.has(header.hash)) {
        Log.debug() << "ARequest:: Dublicated header" << Log.endl;
        return BAD_REQUEST;
    }

    headers.insert(header);
    return CONTINUE;
}

StatusCode
Request::writeBody(const std::string &body) {
    Log.debug() << "Request::writeBody " << body << Log.endl;

    if (body.length() > getBodySize()) {
        Log.error() << "Request: Body length is too long" << Log.endl;
        return BAD_REQUEST;
    }
    setBody(body);
    setFlag(PARSED_BODY);
    return PROCESSING;
}

StatusCode
Request::parseBody(const std::string &line) {
    Log.debug() << "Request::parseBody " << line << Log.endl;
    if (headers.has(TRANSFER_ENCODING)) {
        Log.debug() << "Request::parseChunk" << Log.endl;
        return parseChunk(line);
    } else if (headers.has(CONTENT_LENGTH)) {
        Log.debug() << "Request::writeBody" << Log.endl;
        return writeBody(line);
    }
    return PROCESSING;
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
