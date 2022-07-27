#include "Request.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Location.hpp"

namespace HTTP {

Request::Request(void) 
    : ARequest()
    , _servBlock(NULL)
    , _location(NULL)
    , _client(NULL)
    , _useRanges(true)
    , _authorized(false) {}

Request::Request(Client *client)
    : ARequest()
    , _servBlock(NULL)
    , _location(NULL)
    , _client(client)
    , _useRanges(true)
    , _authorized(false) {}

Request::~Request() {}

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
        _client       = other._client;
        _host         = other._host;
        _useRanges    = other._useRanges;
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

    if (!flagSet(PARSED_SL)) {
        rtrim(line, CRLF);
        setStatus(!line.empty() ? parseSL(line) : CONTINUE);
    } else if (!flagSet(PARSED_HEADERS)) {
        rtrim(line, CRLF);
        setStatus(!line.empty() ? parseHeader(line) : checkHeaders());
    } else if (!flagSet(PARSED_BODY)) {
        setStatus(parseBody(line));

        if (getBody().length() > static_cast<size_t>(getLocation()->getPostMaxBodyRef())) {
            setStatus(PAYLOAD_TOO_LARGE);
        }
    } else {
        setStatus(PROCESSING);
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

bool
Request::tunnelGuard(bool value) {
    if (getClient()->isTunnel() && g_server->settings.blind_proxy) {
        return false;
    }
    return value;
}

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

void
Request::proxyLookUp(void) {
    typedef Proxy::DomainsVec Domains;
    typedef Domains::const_iterator c_iter;

    Log.debug() << "Request::proxyLookUp for \"" << _uri._host << "\"" << Log.endl;
    const Domains &domains = getLocation()->getProxy().getDomainsRef();
    for (c_iter it = domains.begin(); it != domains.end(); ++it) {
        if (*it == _uri._host) {
            Log.debug() << "Request::proxyLookUp: match " << *it << Log.endl;
            isProxy(true);
            return ;
        }
    }
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
        setServerBlock(getClient()->matchServerBlock(_uri._host));
    }
    if (!_location) {
        setLocation(getServerBlock()->matchLocation(_uri._path));
        URI &pass = getLocation()->getProxyRef().getPassRef();
        if (!pass._host.empty() && !pass._port_s.empty()) {
            Log.debug() << "Request::ReverseProxy to " << pass._host << ":" << pass._port << Log.endl;
            isProxy(true);
        } else if (!getClient()->isTunnel()) {
            proxyLookUp();
        }
        resolvePath();
        checkCGI();
    }

    // Call each header handler
    for (Headers<RequestHeader>::iterator it = headers.begin(); it != headers.end(); ++it) {
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

    if (headers.has(CONTENT_LENGTH)) {
        int64_t len;
        bool converted = stoi64(len, headers[CONTENT_LENGTH].value);
        if (!converted) {
            return BAD_REQUEST;
        }
        if (static_cast<std::size_t>(len) > getLocation()->getPostMaxBodyRef()) {
            return PAYLOAD_TOO_LARGE;
        }
    }

    if (!headers.has(TRANSFER_ENCODING) && !headers.has(CONTENT_LENGTH)) {
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
    if (tunnelGuard(headers.has(header.hash))) {
        Log.debug() << "Request:: Dublicated header" << Log.endl;
        return BAD_REQUEST;
    }

    headers.insert(header);
    return CONTINUE;
}

StatusCode
Request::writeBody(const std::string &body) {
    Log.debug() << "Request::writeBody " << body << Log.endl;

    if (tunnelGuard(body.length() > getBodySize())) {
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
    if (headers.has(TRANSFER_ENCODING) && headers.value(TRANSFER_ENCODING) == "chunked") {
        Log.debug() << "Request::parseChunk" << Log.endl;
        return parseChunk(line);
    } else if (headers.has(CONTENT_LENGTH)) {
        Log.debug() << "Request::writeBody" << Log.endl;
        return writeBody(line);
    }
    return PROCESSING;
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

    #ifndef CGI_VDIR
     # define CGI_VDIR "cgi-bin"
    #endif
    
    std::string s = getResolvedPath();

    size_t pos = s.find("/" CGI_VDIR "/");

    if (pos != std::string::npos) {

        s.erase(pos, strlen(CGI_VDIR) + 1);

        const std::vector<std::string> &parts = split(s.substr(pos), "/");
        std::string path = s.substr(0, pos + 1);
        std::string path_info;
        for (size_t i = 0; i < parts.size(); ++i) {
            path += parts[i];
            if (!resourceExists(path)) {
                Log.debug() << "Request:: " << path << " does not exist" << Log.endl; 
                setStatus(NOT_FOUND);
                return ;
            }
            if (!isDirectory(path)) {
                for (size_t j = i + 1; j < parts.size(); j++) {
                    path_info += "/" + parts[j];
                }
                isCGI(true);
                break ;
            }
            path += "/";
        }
        setResolvedPath(path);
        setPathInfo(path_info);
        Log.debug() << "Request:: Upd Path: " << getResolvedPath() << Log.endl;
        Log.debug() << "Request:: PathInfo: " << getPathInfo() << Log.endl;
    } else {
        isCGI(false);
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
    }
    if (!_uri._host.empty()) {
        headers[HOST].value = _uri._host + ":" + _uri._port_s;
    }
    return getMethod() + " " + _uri._path + " " + SERVER_PROTOCOL + CRLF;
}

void
Request::makeHead(void) {

    std::string head;
    head.reserve(512);

    URI &pass = getLocation()->getProxyRef().getPassRef();
    
    head.reserve(512);
    head = makeSL();
    Headers<RequestHeader>::iterator it = headers.begin();
    for (; it != headers.end(); ++it) {
        if (it->first == CONNECTION) {
            if (!pass._host.empty() && !pass._port_s.empty()) {
                head += headerNames[it->first] + ": close" + CRLF;
            }
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

} // namespace HTTP
