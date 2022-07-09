#include "Response.hpp"

#include "CGI.hpp"
#include "Client.hpp"

namespace HTTP {

Response::Response(void)
    : ARequest(), _parsedStatus(OK), _req(NULL), _cgi(NULL), _proxy(NULL), _fileaddr(NULL), _filefd(-1), _isProxy(false), _isCGI(false) {}

Response::Response(Request *req)
    : ARequest(), _parsedStatus(OK), _req(req), _cgi(NULL), _proxy(NULL), _fileaddr(NULL), _filefd(-1), _isProxy(false), _isCGI(false) {
    setStatus(getRequest()->getStatus());
}

Response::Response(const Response &other) {
    *this = other;
}

Response &Response::operator=(const Response &other) {
    if (this != &other) {
        _req = other._req;
        _cgi = other._cgi;
        _proxy = other._proxy;
        _fileaddr = other._fileaddr;
        _filefd = other._filefd;
        _isProxy = other._isProxy;
        _isCGI = other._isCGI;

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
    if (_filefd != -1) {
        close(_filefd);
    }
    if (_fileaddr != NULL) {
        munmap(_fileaddr, _filestat.st_size);
    }
}

void Response::shouldBeClosedIf(void) {
    static const std::size_t size = 5;
    static const StatusCode  failedStatuses[size] = {
         BAD_REQUEST,
         REQUEST_TIMEOUT,
         INTERNAL_SERVER_ERROR,
         PAYLOAD_TOO_LARGE,
         UNAUTHORIZED};

    for (std::size_t i = 0; i < size; i++) {
        if (getStatus() == failedStatuses[i]) {
            Log.debug() << "Response::shouldBeClosedIf" << Log.endl;
            getClient()->shouldBeClosed(true);
            addHeader(CONNECTION, "close");
            break;
        }
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

    _proxy = new Proxy(getRequest()->getLocation()->getProxyRef());

    if (!getClient()->isTunnel()) {
        if (!_proxy->pass(getRequest())) {
            setStatus(BAD_GATEWAY);
        }
    }

    _proxy->prepare(getRequest());
    isProxy(true);
}

void Response::assembleError(void) {
    makeResponseForError();
    shouldBeClosedIf();
    makeHead();
    formed(true);
}

void Response::handle(void) {
    if (getStatus() < BAD_REQUEST) {
        Redirect &rdr = getRequest()->getLocation()->getRedirectRef();
        if (!getRequest()->authorized()) {
            makeResponseForNonAuth();
        } else if (rdr.set()) {
            makeResponseForRedirect(rdr.getCodeRef(), rdr.getURIRef());
        } else if (getClient()->isTunnel() || getRequest()->isProxy()) {
            makeResponseForProxy();
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
    Log.debug() << "Response:: Unauthorized" << Log.endl;
    setStatus(UNAUTHORIZED);
    addHeader(DATE, Time::gmt());
    addHeader(WWW_AUTHENTICATE);
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
    } else if (remove(resourcePath.c_str())) {
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
}

void Response::GET(void) {
    contentForGetHead();
}

void Response::OPTIONS(void) {
    addHeader(ALLOW);
}

void Response::CONNECT(void) {
    _proxy = new Proxy(getRequest()->getLocation()->getProxyRef());

    if (!_proxy->pass(getRequest())) {
        setStatus(BAD_GATEWAY);
    } else {
        getClient()->isTunnel(true);
    }
}

void Response::TRACE(void) {
    setStatus(NOT_IMPLEMENTED);
}

void Response::PATCH(void) {
    setStatus(NOT_IMPLEMENTED);
}

void Response::POST(void) {

    // Parse path_info
    const std::string &resourcePath = _req->getResolvedPath(); 

    if (isDirectory(resourcePath)) {
        // create unique resource name
        // write to file
        return ;
    } else if (isFile(resourcePath)) {
        matchCGI(_req->getResolvedPath());
        if (_cgi != NULL) {
            makeResponseForCGI();
        } else {
            setStatus(BAD_GATEWAY);
        }
    } else {
        setStatus(FORBIDDEN);
        return ; 
    }
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
            writeFile(resourcePath, getRequest()->getBody());
            setBody(DEF_PAGE_BEG "File is overwritten." DEF_PAGE_END);
        }
    } else {
        writeFile(resourcePath, getRequest()->getBody());
        setBody(DEF_PAGE_BEG "File created." DEF_PAGE_END);
    }
    addHeader(CONTENT_TYPE);
}

int Response::makeResponseForDir(void) {
    const std::string &resourcePath = getRequest()->getResolvedPath();

    if (!endsWith(resourcePath, "/")) {
        return makeResponseForRedirect(MOVED_PERMANENTLY, getRequest()->getRawUri() + "/");
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

std::string
Response::getEtagFile(const std::string &filename) {
    struct stat st;
    if (stat(filename.c_str(), &st) < 0) {
        return "";
    }

    SHA1 sha;
    // std::string time = Time::gmt(st.st_mtime);
    // std::string hash = sha.hash(time);
    // Log.debug() << time << " " << hash << Log.endl;

    return sha.hash(Time::gmt(st.st_mtime));
}

int Response::makeResponseForFile(void) {
    const std::string &resourcePath = _req->getResolvedPath();

    matchCGI(resourcePath);
    if (_cgi != NULL) {
        return makeResponseForCGI();
    }

    if (!openFileToResponse(resourcePath)) {
        return 0;
    }

    RangeList &ranges = getRequest()->getRangeList();
    if (ranges.size() == 1) {
        makeResponseForRange();
    } else if (ranges.size() > 1) {
        makeResponseForMultipartRange();
    } else {
        setBody(std::string(_fileaddr, getFileSize()));
        // if (getFileSize() > REGLR_DWNLD_MAX_SIZE) {
        //     addHeader(TRANSFER_ENCODING, "chunked");
        // }
    }

    addHeader(CONTENT_TYPE, getContentType(resourcePath));
    addHeader(ETAG, getEtagFile(resourcePath));
    addHeader(LAST_MODIFIED, Time::gmt(getModifiedTime(resourcePath)));

    return 1;
}

const std::string
Response::getContentRangeValue(RangeSet &range) {
    std::stringstream ss;
    ss << "bytes " << range.to_string() << "/" << getFileSize();
    return ss.str();
}

void Response::makeResponseForMultipartRange(void) {
    const std::string &path = getRequest()->getResolvedPath();
    RangeList         &ranges = getRequest()->getRangeList();

    std::stringstream ss;

    const std::string &sepPrefix = "--";
    const std::string &boundary = "q1w2e3r4";

    RangeList::iterator range = ranges.begin();
    for (range = ranges.begin(); range != ranges.end(); ++range) {
        range->narrow(MAX_RANGE);
        range->rlimit(getFileSize() - 1);

        // Range should not be included if invalid
        ss << sepPrefix << boundary << CRLF;
        ss << headerNames[CONTENT_TYPE] << getContentType(path) << CRLF;
        ss << headerNames[CONTENT_RANGE] << getContentRangeValue(*range) << CRLF;
        ss.write(_fileaddr + range->beg, range->size() - 1);
        ss << CRLF;
    }
    ss << sepPrefix << boundary << sepPrefix << CRLF;

    Log.debug() << "Multipart range processed" << Log.endl;

    setStatus(PARTIAL_CONTENT);
    addHeader(CONTENT_TYPE, "multipart/byteranges; boundary=" + boundary);
    setBody(ss.str());
}

void Response::makeResponseForRange(void) {
    RangeSet &range = getRequest()->getRangeList()[0];

    // MAX_RANGE is about 2MB now
    range.narrow(MAX_RANGE);
    range.rlimit(getFileSize() - 1);

    Log.debug() << "Range processed: " << range.to_string() << ", " << range.size() << Log.endl;

    setStatus(PARTIAL_CONTENT);
    addHeader(CONTENT_RANGE);
    setBody(std::string(_fileaddr + range.beg, range.size()));
}

int Response::openFileToResponse(const std::string &resourcePath) {
    _filefd = open(resourcePath.c_str(), O_RDONLY);
    if (_filefd < 0) {
        setStatus(FORBIDDEN);
        return 0;
    }

    if (fstat(_filefd, &_filestat) < 0) {
        close(_filefd);
        setStatus(FORBIDDEN);
        return 0;
    }

    _fileaddr = (char *)mmap(NULL, _filestat.st_size, PROT_READ, MAP_SHARED, _filefd, 0);
    if (_fileaddr == NULL) {
        close(_filefd);
        setStatus(FORBIDDEN);
        return 0;
    }

    return 1;
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
    return "<a href=\"" + file + "\">" + cutFileName(file) + "</a>";
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
    std::string           body =
        HTML_BEG HEAD_BEG TITLE_BEG + _req->getPath() + TITLE_END META_UTF8 DEFAULT_CSS HEAD_END BODY_BEG H1_BEG "Index on " + _req->getPath() + H1_END HR TABLE_BEG TR_BEG TH_BEG "Filename" TH_END TH_BEG "Last modified" TH_END TH_BEG "Size" TH_END TR_END TR_BEG TD_BEG "<a href=\"..\">../</a>" TD_END TD_BEG TD_END TD_BEG TD_END TR_END;

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
    // setBodyLength(_body.length());

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

    std::string head;
    head.reserve(512);
    head = SERVER_PROTOCOL SP + statusLines[getStatus()] + CRLF;

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
    if (!_cgi->exec(getRequest())) {
        setStatus(BAD_GATEWAY);
        return 0;
    } else {
        isCGI(true);
        return 1;
    }
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

// Rewrite
// void
// Response::makeChunk() {
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
//         _res = itohs(_resourceFileStream.gcount()) + CRLF + _res + CRLF;
//     }
//     if (_resourceFileStream.eof()) {
//         _res += "0" CRLF CRLF;
//     }
// }

Request *
Response::getRequest(void) {
    return _req;
}

Client *
Response::getClient(void) {
    return getRequest()->getClient();
}

// bool Response::isProxy(void) const {
//     return _isProxy;
// }

// void Response::isProxy(bool isProxy) {
//     _isProxy = isProxy;
// }

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

bool Response::parseLine(std::string &line) {
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
        Log.error() << "Somehow we ended up here" << Log.endl;
    }

    if (getStatus() == PROCESSING) {
        if (isCGI() && getBody().empty()) {
            setStatus(NO_CONTENT);
        } else if (isProxy()) {
            setStatus(_parsedStatus);
        } else {
            setStatus(OK);
        }

        makeHead();
        formed(true);
    }

    return formed();
}

bool
Response::tunnelGuard(bool value) {
    if (getClient()->isTunnel() && BLIND_PROXY) {
        return false;
    }
    return value;
}

StatusCode
Response::parseSL(const std::string &line) {
    std::size_t pos = 0;
    setProtocol(getWord(line, " ", pos));
    skipSpaces(line, pos);

    _rawStatus = getWord(line, " ", pos);
    skipSpaces(line, pos);

    // getWord(line, " ", pos);
    // skipSpaces(line, pos);
    // if (line[pos]) {
    //     Log.debug() << "Forbidden symbols at the end of the SL: " << Log.endl;
    //     return BAD_GATEWAY;
    // }

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

    if (tunnelGuard(!header.parse(line, _isProxy))) {
        Log.debug() << "Response:: Invalid header " << line << Log.endl;
        return BAD_REQUEST;
    }

    // dublicate header
    if (tunnelGuard(header.hash != SET_COOKIE)) {
        if (headers.has(header.hash)) {
            headers[header.hash].value += ", " + header.value;
            // Log.debug() << "Response:: Dublicated header " << header.key << " " << header.hash << Log.endl;
            // return BAD_REQUEST;
            return CONTINUE;
        }
    }

    headers.insert(header);
    return CONTINUE;
}

StatusCode
Response::checkHeaders(void) {
    setFlag(PARSED_HEADERS);

    if (headers.has(CONTENT_LENGTH)) {
        std::string &len_s = headers[CONTENT_LENGTH].value;
        long long    num;
        if (!stoll(num, len_s.c_str())) {
            Log.debug() << "Response::ParsedHeaders::Bad length " << num << Log.endl;
            return BAD_GATEWAY;
        }
        setBodySize(num);
    } else if (headers.has(TRANSFER_ENCODING)) {
        // need to add check for chunked responses
        isChunkSize(true);
        setBodySize(0);
    } else {
        setBodySize(ULLONG_MAX);
    }

    headers.erase(CONNECTION);
    headers.erase(KEEP_ALIVE);

    Log.debug() << "Response::ParsedHeaders::Continue" << Log.endl;
    return CONTINUE;
}

StatusCode
Response::writeBody(const std::string &body) {
    Log.debug() << "Response::writeBody " << Log.endl;

    if (body.length() > getBodySize()) {
        Log.error() << "Response: Body length is too long " << Log.endl;
        Log.error() << "Response: expected: " << getBodySize() << Log.endl;
        Log.error() << "Response: got: " << body.length() << Log.endl;
        return BAD_GATEWAY;
    }
    setBody(body);
    setFlag(PARSED_BODY);
    return PROCESSING;
}

StatusCode
Response::parseBody(const std::string &line) {
    Log.debug() << "Response::parseBody " << Log.endl;
    if (headers.has(TRANSFER_ENCODING)) {
        Log.debug() << "Response::parseChunk" << Log.endl;
        return parseChunk(line);
    } else if (headers.has(CONTENT_LENGTH)) {
        Log.debug() << "Response::writeBody" << Log.endl;
        return writeBody(line);
    } else {
        setBody(getBody() + line);
        return CONTINUE;
    }
    return PROCESSING;
}

} // namespace HTTP
