#include "Response.hpp"
#include "CGI.hpp"
#include "Client.hpp"
#include "Server.hpp"

namespace HTTP {

Response::Response(void)
    : ARequest()
    , _parsedStatus(OK)
    , _req(NULL)
    , _cgi(NULL)
    , _proxy(NULL) {}

Response::Response(Request *req)
    : ARequest()
    , _parsedStatus(OK)
    , _req(req)
    , _cgi(NULL)
    , _proxy(NULL) {
    setStatus(getRequest()->getStatus());
    setClient(getRequest()->getClient());
}

Response::Response(const Response &other) {
    *this = other;
}

Response &Response::operator=(const Response &other) {
    if (this != &other) {
        _req = other._req;
        _cgi = other._cgi;
        _proxy = other._proxy;
        headers = other.headers;
    }
    return *this;
}

Response::~Response(void) {
    if (_cgi != NULL) {
        delete _cgi;
    }
    if (_proxy != NULL) {
        delete _proxy;
    }
}

void Response::makeResponseForMethod(void) {
    const std::string &method = getRequest()->getMethod();

    if (method == "GET") {
        GET();
    } else if (method == "PUT") {
        PUT();
    } else if (method == "POST") {
        POST();
    } else if (method == "HEAD") {
        HEAD();
    } else if (method == "PATCH") {
        PATCH();
    } else if (method == "TRACE") {
        TRACE();
    } else if (method == "DELETE") {
        DELETE();
    } else if (method == "OPTIONS") {
        OPTIONS();
    } else if (method == "CONNECT") {
        CONNECT();
    }
}

void Response::makeResponseForProxy() {

    URI &pass = getRequest()->getLocation()->getProxyPassRef();
    URI &uri = getRequest()->getUriRef();
    if (!pass._host.empty() && !pass._port_s.empty()) {
        _proxy = new Proxy(pass._host, pass._port_s);
    } else {
        _proxy = new Proxy(uri._host, uri._port_s);
    }

    if (getRequest()->getFileFd() != -1) {
        if (!getRequest()->mapFile()) {
            setStatus(INTERNAL_SERVER_ERROR);
            return ;
        }

        if (getRequest()->chunked()) {
            getRequest()->addHeader(TRANSFER_ENCODING, "chunked");
        } else {
            getRequest()->parted(true);
        }
    }

    if (!getClient()->isTunnel()) {
        if (!_proxy->pass(getRequest())) {
            setStatus(BAD_GATEWAY);
        }
    }

    getRequest()->makeHead();
    isProxy(true);
}

void Response::assembleError(void) {
    makeResponseForError();
    getClient()->shouldBeClosed(true);
    addHeader(CONNECTION, "close");
    makeHead();
    formed(true);
}

void Response::handle(void) {

    if (getStatus() < 300) {
        isCGI(getRequest()->isCGI());

        Redirect &rdr = getRequest()->getLocation()->getRedirectRef();
        if (!getRequest()->authorized()) {
            makeResponseForNonAuth();
        } else if (rdr.set()) {
            makeResponseForRedirect(rdr.getCodeRef(), rdr.getURIRef());
        } else if (getClient()->isTunnel() || getRequest()->isProxy()) {
            makeResponseForProxy();
        } else if (getRequest()->isCGI()) {
            makeResponseForCGI();
        } else {
            makeResponseForMethod();
        }
    }

    if (getStatus() >= BAD_REQUEST) {
        assembleError();
        return ;
    }

    if (isCGI() || isProxy()) {
        return ;
    }

    makeHead();
    formed(true);
}

void Response::makeResponseForNonAuth(void) {

    if (getRequest()->isProxy()) {
        Log.debug() << "Response:: ProxyAuthenticate" << Log.endl;
        setStatus(PROXY_AUTHENTICATION_REQUIRED);
        addHeader(PROXY_AUTHENTICATE);

    } else {
        Log.debug() << "Response:: Unauthorized" << Log.endl;
        setStatus(UNAUTHORIZED);
        addHeader(WWW_AUTHENTICATE);
    }
    addHeader(DATE, Time::gmt());
}

void Response::DELETE(void) {
    std::string resourcePath = _req->getResolvedPath();

    if (!resourceExists(resourcePath)) {
        setStatus(NOT_FOUND);
        return;
    } else if (isDirectory(resourcePath)) {
        if (rmdirNonEmpty(resourcePath)) {
            setStatus(FORBIDDEN);
            return;
        }
    } else if (std::remove(resourcePath.c_str())) {
        setStatus(FORBIDDEN);
        return;
    }
    setStatus(OK);
    addHeader(CONTENT_TYPE);
    setBody(DEF_PAGE_BEG "File deleted." DEF_PAGE_END);
}

void Response::HEAD(void) {
    contentForGetHead();
    setBody("");
    chunked(false);
    parted(false);
}

void Response::GET(void) {
    contentForGetHead();
}

void Response::OPTIONS(void) {
    addHeader(ALLOW);
}

void Response::CONNECT(void) {

    URI &pass = getRequest()->getLocation()->getProxyPassRef();
    URI &uri = getRequest()->getUriRef();
    if (!pass._host.empty() && !pass._port_s.empty()) {
        _proxy = new Proxy(pass._host, pass._port_s);
    } else {
        _proxy = new Proxy(uri._host, uri._port_s);
    }

    if (!_proxy->pass(getRequest())) {
        setStatus(BAD_GATEWAY);
    } else {
        getClient()->isTunnel(true);
    }
}

void Response::TRACE(void) {

    std::string forwards;

    bool hasForwardHeader = getRequest()->has(MAX_FORWARDS);
    if (hasForwardHeader) {
        forwards = getRequest()->headers[MAX_FORWARDS].value;
    }

    if (!hasForwardHeader || forwards == "0") {
        addHeader(CONTENT_TYPE, "message/http");
        getRequest()->makeHead();
        setBody(getRequest()->getHead());
        setStatus(OK);
    } else {
        Log.error() << "Response::TRACE: Max-forwards " << forwards << "received" << Log.endl;
        setStatus(INTERNAL_SERVER_ERROR);
    }
}

void Response::PATCH(void) {
    setStatus(NOT_IMPLEMENTED);
}

bool
Response::writeBodyToFile(const std::string &resourcePath) {
    if (getRequest()->getFileFd() != -1) {
        if (std::rename(getRequest()->getFilename().c_str(), resourcePath.c_str())) {
            Log.syserr() << "Cannot move tmp file " << _filename << Log.endl;
            return false;
        }
    } else {
        writeFile(resourcePath, getRequest()->getBody());
    }
    return true;
}

void
Response::makeFileWithRandName(const std::string &directory) {

    const std::string &filename = "POST_" + itos(std::rand());
    const std::string &fullpath = directory + "/" + filename;

    if (isFile(fullpath)) {
        setStatus(INTERNAL_SERVER_ERROR);
        return ;
    }

    if (!writeBodyToFile(fullpath)) {
        setStatus(INTERNAL_SERVER_ERROR);
        return ;
    }
    addHeader(LOCATION, getRequest()->getRawUri() + "/" + filename);
    setStatus(CREATED);
}

void Response::POST(void) {

    if (!isDirectory(getRequest()->getResolvedPath())) {
        setStatus(FORBIDDEN);
        return ;
    }

    makeFileWithRandName(getRequest()->getResolvedPath());
}

void Response::PUT(void) {
    const std::string &resourcePath = _req->getResolvedPath();

    if (isDirectory(resourcePath)) {
        setStatus(FORBIDDEN);
        return;
    } else if (isFile(resourcePath)) {
        if (!isWritableFile(resourcePath)) {
            setStatus(FORBIDDEN);
            return;
        } else {
            if (!writeBodyToFile(resourcePath)) {
                setStatus(INTERNAL_SERVER_ERROR);
                return ;
            }
            setBody(DEF_PAGE_BEG "File is overwritten." DEF_PAGE_END);
        }
    } else {
        if (!writeBodyToFile(resourcePath)) {
            setStatus(INTERNAL_SERVER_ERROR);
            return ;
        }
        setBody(DEF_PAGE_BEG "File created." DEF_PAGE_END);
        setStatus(CREATED);
    }
    addHeader(CONTENT_TYPE);
}

int Response::makeResponseForDir(void) {
    const std::string &resourcePath = getRequest()->getResolvedPath();

    if (!endsWith(resourcePath, "/")) {
        std::string redirectPath = getRequest()->getRawUri();
        if (startsWith(redirectPath, "//")) {
            size_t pos = redirectPath.find_first_not_of('/');
            redirectPath.erase(0, pos - 1);
        }
        redirectPath += "/";
        return makeResponseForRedirect(MOVED_PERMANENTLY, redirectPath);
    } else if (indexFileExists(resourcePath)) {
        return makeResponseForFile();
    } else if (getRequest()->getLocation()->getAutoindexRef()) {
        addHeader(CONTENT_TYPE);
        return listing(resourcePath);
    } else {
        setStatus(FORBIDDEN);
        return 0;
    }
    return 1;
}

int Response::contentForGetHead(void) {
    const std::string &resourcePath = _req->getResolvedPath();

    if (!resourceExists(resourcePath)) {
        setStatus(NOT_FOUND);
        return 0;
    }

    if (isDirectory(resourcePath)) {
        return makeResponseForDir();
    } else if (isFile(resourcePath)) {
        return makeResponseForFile();
    } else {
        setStatus(FORBIDDEN);
        return 0;
    }
}

int Response::makeResponseForRedirect(StatusCode code, const std::string &url) {
    setStatus(code);
    addHeader(LOCATION, url);
    addHeader(CONTENT_TYPE);
    setBody(DEF_PAGE_BEG + statusLines[code] + DEF_PAGE_END);

    Log.debug() << "Response:: " << code << " to " << url << Log.endl;
    return 1;
}

bool Response::indexFileExists(const std::string &resourcePath) {
    const std::vector<std::string> &indexes = _req->getLocation()->getIndexRef();
    for (std::size_t i = 0; i < indexes.size(); ++i) {
        std::string path = resourcePath + indexes[i];
        if (isFile(path)) {
            getRequest()->setResolvedPath(path);
            return true;
        }
    }
    return false;
}

int Response::makeResponseForFile(void) {

    const std::string &resourcePath = _req->getResolvedPath();

    Log.debug() << "Response:: " << resourcePath << Log.endl;

    if (!openFile(resourcePath)) {
        Log.error() << "Response:: Cannot open file " << resourcePath << Log.endl; 
        setStatus(INTERNAL_SERVER_ERROR);
        return 0;
    }

    if (!mapFile()) {
        Log.error() << "Response:: Cannot map file " << resourcePath << Log.endl; 
        setStatus(INTERNAL_SERVER_ERROR);
        return 0;
    }

    RangeList &ranges = getRequest()->getRangeList();
    if (ranges.size() == 1) {
        if (!makeResponseForRange()) {
            setStatus(RANGE_NOT_SATISFIABLE);
            return 0;
        }
    } else if (ranges.size() > 1) {
        if (!makeResponseForMultipartRange()) {
            setStatus(RANGE_NOT_SATISFIABLE);
            return 0;
        }
    } else {
        if (static_cast<uint64_t>(getRealBodySize()) > g_server->settings.max_reg_file_size) {
            chunked(true);
        } else {
            setBody(std::string(_fileaddr, getFileSize()));
        }
    }

    addHeader(CONTENT_TYPE, getContentType(resourcePath));

    if (!resourcePath.empty()) {
        ETag *etag = ETag::get(resourcePath);
        etag->setTag(_filestat.st_mtime);

        addHeader(ETAG, etag->getTag());
        addHeader(LAST_MODIFIED, etag->getEntityStrTime());
    }

    if (chunked()) {
        addHeader(TRANSFER_ENCODING, "chunked");
    } else {
        addHeader(CONTENT_LENGTH);
    }

    return 1;
}

const std::string
Response::getContentRangeValue(RangeSet &range) {
    std::stringstream ss;
    ss << "bytes " << range.to_string() << "/" << getFileSize();
    return ss.str();
}

int
Response::makeResponseForMultipartRange(void) {
    const std::string &path = getRequest()->getResolvedPath();
    RangeList         &ranges = getRequest()->getRangeList();

    std::stringstream ss;

    const std::string &sepPrefix = "--";
    const std::string &boundary = Base64::encode(SHA1().hash(itos(std::rand())));

    RangeList::iterator range = ranges.begin();
    for (range = ranges.begin(); range != ranges.end(); ++range) {
        range->narrow(g_server->settings.max_range_size);
        range->rlimit(getFileSize() - 1);

        if (range->beg > getFileSize()) {
            continue ;
        }

        // Range should not be included if invalid
        ss << sepPrefix << boundary << CRLF;
        ss << headerNames[CONTENT_TYPE] << ":" << getContentType(path) << CRLF;
        ss << headerNames[CONTENT_RANGE] << ":" << getContentRangeValue(*range) << CRLF;
        ss.write(_fileaddr + range->beg, range->size() - 1);
        ss << CRLF;
    }
    ss << sepPrefix << boundary << sepPrefix << CRLF;

    Log.debug() << "Multipart range processed" << Log.endl;

    setStatus(PARTIAL_CONTENT);
    addHeader(CONTENT_TYPE, "multipart/byteranges; boundary=" + boundary);
    setBody(ss.str());

    return 1;
}

int Response::makeResponseForRange(void) {
    RangeSet &range = getRequest()->getRangeList()[0];

    range.narrow(g_server->settings.max_range_size);
    range.rlimit(getFileSize() - 1);

    if (range.beg > getFileSize()) {
        return 0;
    }

    Log.debug() << "Range processed: " << range.to_string() << ", " << range.size() << Log.endl;

    setStatus(PARTIAL_CONTENT);
    addHeader(CONTENT_RANGE);
    setBody(std::string(_fileaddr + range.beg, range.size()));

    return 1;
}

int Response::openFile(const std::string &resourcePath) {

    _filefd = open(resourcePath.c_str(), O_RDONLY);
    return (_filefd != -1);
}

std::string
cutFileName(const std::string &file) {
    int maxFilenameLen = 30;
    int u8BytesCount = 2;

    std::string line;
    line.reserve(maxFilenameLen * u8BytesCount);

    int curFilenameLen = file.length();
    int curFilenameLenU8 = strlen_u8(file);

    if (curFilenameLen != curFilenameLenU8) {
        // UTF could contain more than 2 bytes
        maxFilenameLen *= u8BytesCount;
    }
    line = file.substr(0, maxFilenameLen);

    if (curFilenameLenU8 > maxFilenameLen) {
        line[line.length() - 1] = '.';
        line[line.length() - 2] = '.';
        line[line.length() - 3] = '.';
    }

    return line;
}

int Response::fillFileStat(const std::string &filename, struct stat *st) {
    const std::string &fullname = getRequest()->getResolvedPath() + '/' + filename;

    if (stat(fullname.c_str(), st) < 0) {
        Log.debug() << "cannot get stat of " << fullname << Log.endl;
        return 0;
    }
    return 1;
}

static std::string
getHumanSize(long long bytes, bool si = true) {
    static const char *suffixes = " kMGTPE";

    const char *end = (!si && bytes > 1024) ? "iB" : "B";
    int         base = si ? 1000 : 1024;

    if (bytes < base) {
        return lltos(bytes) + suffixes[0] + end;
    }

    int    index = static_cast<int>(log(bytes) / log(base));
    double humanSize = static_cast<double>(bytes / pow(base, index));
    char   buf[10] = {0};
    snprintf(buf, 10, "%.1f%c%s", humanSize, suffixes[index], end);
    return buf;
}

static inline std::string
createLinkLine(const std::string &file) {
    std::string trail;
    if (isDirectory(file) && !endsWith(file, "/")) {
        trail = "/";
    }
    return "<a href=\"" + file + trail + "\">" + cutFileName(file) + "</a>";
}

std::string
Response::createTableLine(const std::string &filename) {
    std::string line;
    line.reserve(512);

    struct stat st;
    if (!fillFileStat(filename, &st)) {
        return "";
    }

    line += TR_BEG;
    line += TD_BEG + createLinkLine(filename) + TD_END;
    line += TD_BEG + Time::gmt("%d/%m/%Y %H:%m", st.st_mtime) + TD_END;
    line += TD_BEG + getHumanSize(st.st_size) + TD_END;
    line += TR_END;

    return line;
}

int Response::listing(const std::string &resourcePath) {
    std::string body =
    HTML_BEG
        HEAD_BEG 
            TITLE_BEG + _req->getPath() + TITLE_END
            META_UTF8 DEFAULT_CSS 
        HEAD_END
        BODY_BEG 
        H1_BEG "Index on " + _req->getPath() + H1_END
        HR TABLE_BEG 
            TR_BEG TH_BEG "Filename" TH_END TH_BEG "Last modified" TH_END TH_BEG "Size" TH_END TR_END
            TR_BEG TD_BEG "<a href=\"../\">../</a>" TD_END TD_BEG TD_END TD_BEG TD_END TR_END;

    std::deque<std::string> filenames;
    if (!fillDirContent(filenames, resourcePath)) {
        return 0;
    }

    std::sort(filenames.begin(), filenames.end());
    std::deque<std::string>::iterator it;
    for (it = filenames.begin(); it != filenames.end(); ++it) {
        if (*it == "." || *it == "..") {
            continue;
        }
        body += createTableLine(*it);
    }
    body += TABLE_END HR BODY_END HTML_END;

    setBody(body);

    return 1;
}

int Response::fillDirContent(std::deque<std::string> &filenames, const std::string &dirName) {
    DIR *dir = opendir(dirName.c_str());
    if (!dir) {
        setStatus(INTERNAL_SERVER_ERROR);
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        filenames.push_back(entry->d_name);
    }

    closedir(dir);
    return 1;
}

std::string
Response::getContentType(const std::string &resourcePath) {
    typedef std::map<std::string, std::string>::const_iterator c_iter;

    std::size_t pos = resourcePath.find_last_of('.');
    if (pos != std::string::npos) {
        c_iter it = MIMEs.find(resourcePath.substr(pos + 1));
        if (it != MIMEs.end()) {
            return it->second;
        }
    }
    return "text/plain";
}

void Response::makeHead(void) {

    // Mandatory headers
    addHeader(DATE);
    addHeader(SERVER);
    addHeader(KEEP_ALIVE);
    addHeader(CONNECTION);
    addHeader(CONTENT_TYPE);
    addHeader(CONTENT_LENGTH);
    addHeader(ACCEPT_RANGES);

    Log.info() << getRequest()->getMethod() << " " << getRequest()->getUriRef()._path << " = " << getStatus() << Log.endl;

    std::string head;
    head.reserve(512);
    head = SERVER_PROTOCOL SP + statusLines[getStatus()] + CRLF;
    
    std::map<std::string, std::string> clientCookie = getRequest()->getCookie();
    if (clientCookie.find("s_id") == clientCookie.end()) {
        Cookie s_id("s_id", SHA1().hash(itos(rand())));
        s_id.httpOnly = g_server->settings.cookie_httpOnly;
        s_id.maxAge = g_server->settings.session_lifetime;
        s_id.setPath("/");
        std::string sidStr = s_id.toString();
        g_server->addSession(sidStr);
        addHeader(SET_COOKIE, sidStr);
    }

    Headers<ResponseHeader>::iterator it;
    for (it = headers.begin(); it != headers.end(); ++it) {
        if (it->second.value.empty()) {
            it->second.handle(*this);
        }
        if (!it->second.value.empty()) {
            head += headerNames[it->second.hash] + ": " + it->second.value + CRLF;
        }
    }

    Headers<ResponseHeader> &add_headers = getRequest()->getLocation()->getHeaders();
    Headers<ResponseHeader>::iterator it_addh;
    for (it_addh = add_headers.begin(); it_addh != add_headers.end(); ++it_addh) {
        if (!it_addh->second.value.empty()) {
            head += headerNames[it_addh->second.hash] + ": " + it_addh->second.value + CRLF;
        }
    }

    head += CRLF;
    setHead(head);
}

void Response::addHeader(uint32_t hash, const std::string &value) {
    if (headers[hash].value.empty()) {
        headers[hash].value = value;
    }
}

int Response::makeResponseForCGI(void) {

    Location::MethodsVec &allowed = getRequest()->getLocation()->getCGIMethodsRef();
    if (std::find(allowed.begin(), allowed.end(), getRequest()->getMethod()) == allowed.end()) {
        setStatus(METHOD_NOT_ALLOWED);
        return 0;
    }

    if (!resourceExists(_req->getResolvedPath())) {
        setStatus(NOT_FOUND);
        return 0;   
    }

    if (!isFile(_req->getResolvedPath())) {
        setStatus(FORBIDDEN);
        return 0;
    }

    matchCGI(_req->getResolvedPath());

    if (_cgi == NULL) {
        setStatus(BAD_GATEWAY);
        return 0;
    }

    if (getRequest()->getFileFd() != -1) {
        if (!getRequest()->mapFile()) {
            setStatus(INTERNAL_SERVER_ERROR);
            return 0;
        }
        getRequest()->parted(true);
    }

    if (!_cgi->exec(getRequest())) {
        setStatus(BAD_GATEWAY);
        return 0;
    }

    return 1;
}

void Response::makeResponseForError(void) {
    if (getRequest()->getLocation() != NULL) {
        std::map<int, std::string>          &pages = getRequest()->getLocation()->getErrorPagesRef();
        std::map<int, std::string>::iterator it = pages.find(getStatus());

        if (it != pages.end()) {
            setBody(readFile(it->second));
            if (!getBody().empty()) {
                return;
            }
        }
    }

    if (!errorResponses.has(getStatus())) {
        Log.error() << "Unknown response code: " << static_cast<int>(getStatus()) << Log.endl;
        setStatus(UNKNOWN_ERROR);
    }
    setBody(errorResponses[getStatus()]);
}

Request *
Response::getRequest(void) {
    return _req;
}

void *
Response::getFileAddr(void) {
    return _fileaddr;
}

int64_t
Response::getFileSize(void) {
    return _filestat.st_size;
}

CGI *Response::getCGI(void) const {
    return _cgi;
}

void Response::setCGI(CGI *cgi) {
    _cgi = cgi;
}

void Response::matchCGI(const std::string &filepath) {
    typedef std::map<std::string, CGI> cgisMap;
    typedef cgisMap::iterator          iter;

    if (!isCGI()) {
        return ;
    }

    cgisMap &cgis = getRequest()->getLocation()->getCGIsRef();
    for (iter it = cgis.begin(); it != cgis.end(); it++) {
        if (endsWith(filepath, it->first)) {
            setCGI(new CGI(it->second));
            if (getCGI() == NULL) {
                Log.syserr() << "Cannot allocate memory for CGI" << Log.endl;
            }
            getCGI()->setScriptPath(filepath);
        }
    }
}

void Response::checkCGIFailure(void) {
    int status;
    if (waitpid(getCGI()->getPID(), &status, WNOHANG) > 0) {
        if (WEXITSTATUS(status) != 0) {
            setStatus(BAD_GATEWAY);
        }
    }
}

bool
Response::has(uint32_t hash) {
    return headers.has(hash);
}

bool Response::parseLine(std::string &line) {
    // Log.debug() << "Response::parseLine:: " << line << Log.endl;

    if (!isCGI() && !flagSet(PARSED_SL)) {
        rtrim(line, CRLF);
        setStatus(!line.empty() ? parseSL(line) : CONTINUE);
    } else if (!flagSet(PARSED_HEADERS)) {
        rtrim(line, CRLF);
        setStatus(!line.empty() ? parseHeader(line) : checkHeaders());
    } else if (!flagSet(PARSED_BODY)) {
        setStatus(!line.empty() ? parseBody(line) : PROCESSING);
    } else {
        setStatus(INTERNAL_SERVER_ERROR);
        Log.error() << "Response:: Formed flag wasn't set" << Log.endl;
    }

    if (getStatus() == PROCESSING) {
        if (isCGI() && getRealBodySize() == 0) {
            setStatus(NO_CONTENT);
        } else if (isProxy()) {
            setStatus(_parsedStatus);
        } else {
            setStatus(OK);
        }

        if (isCGI() && has(LOCATION)) {
            setStatus(SEE_OTHER);
        }

        if (getFileFd() != -1) {
            if (!mapFile()) {
                setStatus(INTERNAL_SERVER_ERROR);
            } else if (!chunked()) {
                parted(true);
            }
        }

        makeHead();
        formed(true);
    }

    return formed();
}

StatusCode
Response::parseSL(const std::string &line) {
    std::size_t pos = 0;
    setProtocol(getWord(line, " ", pos));
    skipSpaces(line, pos);

    _rawStatus = getWord(line, " ", pos);
    skipSpaces(line, pos);

    return checkSL();
}

StatusCode
Response::checkSL(void) {
    if (tunnelGuard(!isValidProtocol(getProtocol()))) {
        Log.debug() << "Response::checkSL: protocol " << getProtocol() << " is not valid" << Log.endl;
        return BAD_GATEWAY;
    }
    if (tunnelGuard(!getProtocol().empty())) {
        setMajor(getProtocol()[5] - '0');
        setMinor(getProtocol()[7] - '0');
    }
    if (tunnelGuard(getMajor() != _req->getMajor())) {
        Log.debug() << "Response::checkSL: protocol " << getProtocol() << " mismatch" << Log.endl;
        return BAD_GATEWAY;
    }

    long long status;
    stoll(status, _rawStatus.c_str());

    if (tunnelGuard(status < 200 || status > 599)) {
        Log.debug() << "Response::checkSL: status code " << status << " is invalid" << Log.endl;
        return BAD_GATEWAY;
    }
    _parsedStatus = static_cast<StatusCode>(status);

    setFlag(PARSED_SL);
    return CONTINUE;
}

StatusCode
Response::parseHeader(const std::string &line) {
    ResponseHeader header;

    // Log.debug() << "Response::parseHeader:: " << line << Log.endl;
    if (tunnelGuard(!header.parse(line, isProxy()))) {
        Log.debug() << "Response:: Invalid header " << line << Log.endl;
        return BAD_REQUEST;
    }

    // dublicate header
    if (tunnelGuard(header.hash != SET_COOKIE)) {
        if (has(static_cast<HeaderCode>(header.hash))) {
            headers[header.hash].value += ", " + header.value;
            return CONTINUE;
        }
    }

    headers.insert(header);
    return CONTINUE;
}

StatusCode
Response::checkHeaders(void) {
    setFlag(PARSED_HEADERS);

    if (has(CONTENT_LENGTH)) {
        std::string &len_s = headers[CONTENT_LENGTH].value;
        long long    num = -1;
        if (!stoll(num, len_s.c_str()) || num < 0) {
            Log.debug() << "Response:: Invalid Content-length" << num << Log.endl;
            return BAD_GATEWAY;
        }
        setExpBodySize(num);
    } else if (has(TRANSFER_ENCODING)) {
        if (headers[TRANSFER_ENCODING].value == "chunked") {
            isChunkSize(true);
            chunked(true);
            headers.erase(TRANSFER_ENCODING);
        }
    } else {
        setExpBodySize(ULLONG_MAX);
    }

    headers.erase(SERVER);
    headers.erase(CONNECTION);
    headers.erase(KEEP_ALIVE);

    Log.debug() << "Response::ParsedHeaders::Continue" << Log.endl;
    return CONTINUE;
}

} // namespace HTTP
