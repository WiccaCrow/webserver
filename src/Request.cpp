#include "Request.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Location.hpp"

namespace HTTP {

Request::Request(void) 
    : ARequest()
    , _servBlock(NULL)
    , _location(NULL)
    , _useRanges(true)
    , _authorized(false) {}

Request::Request(Client *client)
    : ARequest()
    , _servBlock(NULL)
    , _location(NULL)
    , _useRanges(true)
    , _authorized(false) {
    setClient(client);
}

Request::~Request(void) {
}

Request::Request(const Request &other) : ARequest(other) {
    *this = other;
}

Request &
Request::operator=(const Request &other) {
    if (this != &other) {
        ARequest::operator=(other);
        _method       = other._method;
        _uri          = other._uri;
        _rawURI       = other._rawURI;
        _resolvedPath = other._resolvedPath;
        _servBlock    = other._servBlock;
        _location     = other._location;
        _authorized   = other._authorized;
        _cookie       = other._cookie;
        _host         = other._host;
        _useRanges    = other._useRanges;
        headers       = other.headers;
    }
    return *this;
}

const std::string &
Request::getMethod() const {
    return _method;
}

const std::string &
Request::getPath(void) const {
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

const std::string &
Request::getRemoteUser(void) const {
    return _remoteUser;
}

void
Request::setRemoteUser(const std::string &user) {
    _remoteUser = user;
}

const std::string &
Request::getPathInfo(void) const {
    return _pathInfo;
}

void
Request::setPathInfo(const std::string &info) {
    _pathInfo = info;
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

RangeList &
Request::getRangeList(void) {
    return _ranges;
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

bool
Request::useRanges(void) const {
    return _useRanges;
}

void
Request::useRanges(bool flag) {
    _useRanges = flag;
}

bool
Request::parseLine(std::string &line) {

    if (getStatus() < BAD_REQUEST) {

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
    }

    if (getStatus() != CONTINUE) {
        if (!_servBlock) {
            Log.debug() << "Serverblock is not set after parsing. Default matching" << Log.endl;
            setServerBlock(getClient()->matchServerBlock(_host._host));
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

// bool
// Request::tunnelGuard(bool value) {
//     if (getClient()->isTunnel() && g_server->settings.blind_proxy) {
//         return false;
//     }
//     return value;
// }

StatusCode
Request::parseSL(const std::string &line) {

    Log.debug() << line << Log.endl;

    std::size_t pos = 0;
    _method = getWord(line, " ", pos);

    skipSpaces(line, pos);
    _rawURI = getWord(line, " ", pos);
    if (_rawURI.length() > g_server->settings.max_uri_length) {
        Log.debug() << "Request:: URI is too long: " << _rawURI << Log.endl;
        return URI_TOO_LONG;
    }

    _uri.parse(_rawURI);

    skipSpaces(line, pos);
    setProtocol(getWord(line, " ", pos));

    skipSpaces(line, pos);

    if (tunnelGuard(line[pos])) {
        Log.debug() << "Request:: Forbidden symbols at the end of the SL: " << Log.endl;
        return BAD_REQUEST;
    }
    return checkSL();
}

StatusCode
Request::checkSL(void) {
    if (tunnelGuard(!isValidMethod(_method))) {
        Log.debug() << "Request::parseSL: Method " << getMethod() << " is not implemented" << Log.endl;
        return BAD_REQUEST;
    }

    // if (isValidPath(_rawURI)) {
    //     Log.debug() << "Invalid URI" << Log.endl;
    //     return BAD_REQUEST;
    // }

    if (checkDirDepth(_uri._path) < 0) {
        Log.debug() << "Request::checkSL: uri " << _uri._path << " is forbidden" << Log.endl;
        return FORBIDDEN;
    }

    if (tunnelGuard(!_uri._host.empty())) {

        if (!_servBlock) {
            setServerBlock(getClient()->matchServerBlock(_uri._host));
        }
        if (!isValidProxyDomain(_uri._host) && !g_server->isServerHostname(_uri._host)) {
            Log.debug() << "Request:: Invalid hostname " << _uri._host << " in SL" << Log.endl;
            return BAD_REQUEST;
        }
    }

    if (tunnelGuard(!isValidProtocol(getProtocol()))) {
        Log.debug() << "Request::checkSL: protocol " << getProtocol() << " is not valid" << Log.endl;
        return BAD_REQUEST;
    }
    if (tunnelGuard(!getProtocol().empty())) {
        setMajor(getProtocol()[5] - '0');
        setMinor(getProtocol()[7] - '0');
    }
    if (tunnelGuard(getMajor() > 1)) {
        Log.debug() << "Request::checkSL: protocol " << getProtocol() << " is not supported" << Log.endl;
        return HTTP_VERSION_NOT_SUPPORTED;
    } else if (tunnelGuard(getMajor() != 1 || getMinor() != 1)) {
        Log.debug() << "Request::checkSL: protocol " << getProtocol() << " is not implemented" << Log.endl;
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

bool
Request::isValidProxyDomain(const std::string &name) {
    typedef ServerBlock::DomainsVec Domains;
    typedef Domains::const_iterator c_iter;

    const Domains &domains = getServerBlock()->getProxyDomainsRef();
    for (c_iter it = domains.begin(); it != domains.end(); ++it) {
        if (*it == name) {
            isProxy(true);
            return true;
        }
    }
    return false;
}

void
Request::checkReverseProxy(void) {

    URI &pass = getLocation()->getProxyPassRef();
    if (!pass._host.empty() && !pass._port_s.empty()) {
        Log.debug() << "Request:: reverse proxy to " << pass._host << ":" << pass._port << Log.endl;
        isProxy(true);
    }
}

bool
Request::has(uint32_t hash) {
    return headers.has(hash);
}

StatusCode
Request::checkHeaders(void) {

    setFlag(PARSED_HEADERS);

    bool hasHost = has(HOST);
    if (_uri._host.empty() && !hasHost) {
        Log.debug() << "Request:: host is not present" << Log.endl;
        return BAD_REQUEST;
    }

    if (has(REFERER)) {
        headers[REFERER].handle(*this);
    }

    if (hasHost) {
        RequestHeader &host = headers[HOST];
        if (tunnelGuard(host.handle(*this) != CONTINUE)) {
            return BAD_REQUEST;
        }
    }
    
    if (!_servBlock) {
        setServerBlock(getClient()->matchServerBlock(_uri._host));
    }

    if (!_location) {
        setLocation(getServerBlock()->matchLocation(_uri._path));
    }

    if (_location) {
        if (g_server->isServerHostname(_uri._host)) {
            checkReverseProxy();
            if (!isProxy()) {
                checkCGI();
            }
        } else if (!isValidProxyDomain(_uri._host)) {
            Log.debug() << "Request:: Invalid hostname " << _uri._host << Log.endl;
            return BAD_REQUEST;
        }
    
        resolvePath();
    }

    // Call each header handler
    for (Headers<RequestHeader>::iterator it = headers.begin(); it != headers.end(); ++it) {
        if (it->first == HOST || it->first == REFERER) {
            continue;
        }
    
        StatusCode status = it->second.handle(*this);
        if (tunnelGuard(status != CONTINUE)) {
            // Log.debug() << it->second.key << ": " << it->second.value << Log.endl;
            return status;
        }
    }

    Location::MethodsVec &allowed = getLocation()->getAllowedMethodsRef();
    Location::MethodsVec::iterator it_method = std::find(allowed.begin(), allowed.end(), _method);
    if (tunnelGuard(it_method == allowed.end())) {
        Log.debug() << "Request:: Method " << getMethod() << " is not allowed" << Log.endl;
        return METHOD_NOT_ALLOWED;
    }

    if (has(TRANSFER_ENCODING)) {
        headers.erase(TRANSFER_ENCODING);
    }

    if (!chunked() && !has(CONTENT_LENGTH)) {
        // PUT or POST or PATCH
        if (tunnelGuard(_method[0] == 'P')) {
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

    if (tunnelGuard(!header.parse(line))) {
        Log.debug() << "Request:: Invalid header " << line << Log.endl;
        return BAD_REQUEST;
    }

    if (tunnelGuard(header.value.length() > g_server->settings.max_header_field_length)) {
        Log.debug() << "Request:: Header is too large: " << line << Log.endl;
        return REQUEST_HEADER_FIELDS_TOO_LARGE;
    }

    // dublicate header
    if (tunnelGuard(has(header.hash))) {
        Log.debug() << "Request:: Dublicated header" << Log.endl;
        return BAD_REQUEST;
    }

    headers.insert(header);
    return CONTINUE;
}

std::map<std::string, std::string> &
Request::getCookie(void) {
    return _cookie;
}

void
Request::setCookie(std::map<std::string, std::string> cookie) {
    _cookie = cookie;
}

void
Request::checkCGI(void) {

    #ifndef CGI_DIR
     # define CGI_DIR "cgi-bin"
    #endif

    Location::CGIsMap &cgis = getLocation()->getCGIsRef();
    
    bool hasCGIDir = false;
    bool hasScript = false;

    std::string path;
    std::string path_info;

    char *str = strdup(_uri._path.c_str());
    if (str == NULL) {
        return ;
    }

    char *token = strtok(str, "/");
    while (token) {

        if (hasCGIDir && hasScript) {
            path_info += "/";
            path_info += token;
        } else {
            path += "/";
            path += token; 
        }

        if (!hasCGIDir && !strcmp(token, CGI_DIR)) {
            hasCGIDir = true;
        } else if (hasCGIDir) {
            Location::CGIsMap::iterator it;
            for (it = cgis.begin(); it != cgis.end(); ++it) {
                if (endsWith(token, it->first)) {
                    hasScript = true;
                }
            }
        }
        token = strtok(NULL, "/");
    }

    free(str);

    if (hasCGIDir && hasScript) {
        Log.debug() << "Request:: CGI-request: " << path << Log.endl;
        isCGI(true);

        Log.debug() << "Request:: PathInfo: " << path_info << Log.endl;
        setPathInfo(path_info);
        _uri._path = path;
    }
}

std::string
Request::makeSL(void) {

    if (_uri._path.empty()) {
        if (getMethod() == "OPTIONS") {
            _uri._path = "*";
        } else {
            _uri._path = "/";
        }
    } else {
        const std::string &locpath = getLocation()->getPathRef();
        if (locpath != "/") {
            std::size_t pos = _uri._path.find(locpath);
            if (pos == std::string::npos) {
                Log.error() << "Location path " << locpath <<  " not found in the uri path " << _uri._path << Log.endl;
            } else {
                _uri._path.erase(pos, pos + locpath.length());
            }

            if (!startsWith(_uri._path, "/")) {
                _uri._path = "/" + _uri._path;
            }
        }
    }

    URI &pass = getLocation()->getProxyPassRef();
    if (!pass._host.empty()) {
        headers[HOST].value = pass._host + ":" + (pass._port_s.empty() ? "80" : pass._port_s);
    } else if (!_uri._host.empty()) {
        headers[HOST].value = _uri._host + ":" + (_uri._port_s.empty() ? "80" : _uri._port_s);
    }
    return getMethod() + " " + _uri._path + " " + SERVER_PROTOCOL + CRLF;
}

void
Request::makeHead(void) {

    std::string head;
    head.reserve(512);

    head = makeSL();
    Headers<RequestHeader>::iterator it = headers.begin();
    for (; it != headers.end(); ++it) {
        if (it->first == CONNECTION) {
            continue;
        }
        else if (it->first == KEEP_ALIVE) {
            continue;
        }
        head += it->second.toString() + CRLF;
    }
    head += CRLF;
    setHead(head);
}

void
Request::addHeader(uint32_t hash, const std::string &value) {
    if (headers[hash].value.empty()) {
        headers[hash].value = value;
    }
}

} // namespace HTTP
