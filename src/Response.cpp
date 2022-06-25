#include "Response.hpp"
#include "CGI.hpp"
#include "Client.hpp"

namespace HTTP {

Response::Response() 
    : _req(NULL)
    , _client(NULL)
    , _cgi(NULL)
    , _bodyLength(0)
    , _isFormed(false)
    , _fileaddr(NULL) 
    , _filefd(-1) {}

Response::Response(Request *req) 
    : _req(req)
    , _client(NULL)
    , _cgi(NULL)
    , _bodyLength(0)
    , _isFormed(false)
    , _status(req->getStatus())
    , _fileaddr(NULL)
    , _filefd(-1)
{
    headers.push_back(ResponseHeader(DATE));
    headers.push_back(ResponseHeader(SERVER));
    headers.push_back(ResponseHeader(KEEP_ALIVE));
    headers.push_back(ResponseHeader(CONNECTION));
    headers.push_back(ResponseHeader(CONTENT_TYPE));
    headers.push_back(ResponseHeader(CONTENT_LENGTH));
    headers.push_back(ResponseHeader(ACCEPT_RANGES));

}

Response::Response(const Response &other) {
    *this = other;
}

Response &Response::operator=(const Response &other) {
    if (this != &other) {
        _res = other._res;
        _resLeftToSend = other._resLeftToSend;
        _req = other._req;
        _client = other._client;
        _cgi = other._cgi;
        _body = other._body;
        _extraHeaders = other._extraHeaders;
        _bodyLength = other._bodyLength;
        _isFormed = other._isFormed;
        _status = other._status;
        headers = other.headers;
        _fileaddr = other._fileaddr;
        _filefd = other._filefd;
    }
    return *this;
}

Response::~Response() { 
    if (getRequest() != NULL) {
        delete getRequest();
    } 
    if (_filefd != -1) {
        close(_filefd);
    }
    if (_fileaddr != NULL) {
        munmap(_fileaddr, _filestat.st_size);
    }
}

void
Response::shouldBeClosedIf(void) {

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
Response::performMethod(void) {

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

void
Response::handle(void) {

    if (getStatus() < BAD_REQUEST) {
    
        if (getRequest()->authNeeded() && !getRequest()->isAuthorized()) {
            makeResponseForNonAuth();
        } else if (getRequest()->getLocation()->getRedirectRef().set()) {
            Redirect &rdr = getRequest()->getLocation()->getRedirectRef();
            makeResponseForRedirect(rdr.getCodeRef(), rdr.getURIRef());
        } else {
            performMethod();
        }
    }
    
    if (getStatus() >= BAD_REQUEST) {
        makeResponseForError();
        shouldBeClosedIf();
        addHeader(CONTENT_TYPE); // ?
    }

    makeHead();
    isFormed(true);
}

void
Response::makeResponseForNonAuth(void) {
    Log.debug() << "Response:: Unauthorized" << Log.endl;
    setStatus(UNAUTHORIZED);
    addHeader(DATE, Time::gmt());
    addHeader(WWW_AUTHENTICATE);
}

void
Response::DELETE(void) {
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
    setBody(HTML_BEG BODY_BEG H1_BEG
            "File deleted."
            H1_END BODY_END HTML_END);
}

void
Response::HEAD(void) {
    contentForGetHead();
    _body = "";
}

void
Response::GET(void) {
    contentForGetHead();
}

void
Response::OPTIONS(void) {
    addHeader(ALLOW);
}

void
Response::CONNECT(void) {
    setStatus(NOT_IMPLEMENTED);
}

void
Response::TRACE(void) {
    setStatus(NOT_IMPLEMENTED);
}

void
Response::PATCH(void) {
    setStatus(NOT_IMPLEMENTED);
}

void
Response::POST(void) {
    std::map<std::string, CGI>::iterator it = isCGI(_req->getResolvedPath());
    if (it != _req->getLocation()->getCGIsRef().end()) {
        makeResponseForCGI(it->second);
    } else {
        setStatus(NO_CONTENT);
    }
}

void
Response::PUT(void) {
    std::string resourcePath = _req->getResolvedPath();
    if (isDirectory(resourcePath)) {
        setStatus(FORBIDDEN);
        return ;
    } else if (isFile(resourcePath)) {
        if (!isWritableFile(resourcePath)) {
            setStatus(FORBIDDEN);
            return ;
        } else {
            writeFile(resourcePath);
            setBody(HTML_BEG BODY_BEG H1_BEG
                    "File is overwritten."
                    H1_END BODY_END HTML_END);
        }
    } else {
        writeFile(resourcePath);
        setBody(HTML_BEG BODY_BEG H1_BEG
                "File created."
                H1_END BODY_END HTML_END);
    }
    addHeader(CONTENT_TYPE);
}

int
Response::makeResponseForDir(std::string &resourcePath) {
    if (!endsWith(resourcePath, "/")) {
        return makeResponseForRedirect(MOVED_PERMANENTLY, _req->getRawUri() + "/");
    } else if (isSetIndexFile(resourcePath)) {
        return makeResponseForFile(resourcePath);
    } else if (_req->getLocation()->getAutoindexRef()) {
        addHeader(CONTENT_TYPE);
        return listing(resourcePath);
    } else {
        // autoindex off
        setStatus(FORBIDDEN);
        return 0;
    }
    return 1;
}

int
Response::contentForGetHead(void) {
    std::string resourcePath = _req->getResolvedPath();

    if (!resourceExists(resourcePath)) {
        setStatus(NOT_FOUND);
        return 0;
    }

    if (isDirectory(resourcePath)) {
        return makeResponseForDir(resourcePath);
    } else if (isFile(resourcePath)) {
        return makeResponseForFile(resourcePath);
    } else {
        setStatus(FORBIDDEN);
        return 0;
    }
}

int
Response::makeResponseForRedirect(HTTP::StatusCode code, const std::string &url) {

    setStatus(code);
    addHeader(LOCATION, url);
    addHeader(CONTENT_TYPE);
    setBody(HTML_BEG BODY_BEG H1_BEG
            "Redirect " + ultos(code) +
            H1_END BODY_END HTML_END);

    Log.debug() << "Response:: " << code << " to " << url << Log.endl;
    return 1;
}

bool
Response::isSetIndexFile(std::string &resourcePath) {
    const std::vector<std::string> &indexes = _req->getLocation()->getIndexRef();
    for (size_t i = 0; i < indexes.size(); ++i) {
        std::string path = resourcePath + indexes[i];
        if (isFile(path)) {
            resourcePath = path;
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

    return SHA1(Time::gmt(st.st_mtime));
}

int
Response::makeResponseForFile(const std::string &resourcePath) {
    std::map<std::string, CGI>::iterator it = isCGI(resourcePath);
    if (it != _req->getLocation()->getCGIsRef().end()) {
        return makeResponseForCGI(it->second);
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
        // if (getFileSize() > REGLR_DWNLD_MAX_SIZE) {
        //     addHeader(TRANSFER_ENCODING, "chunked");
        // }
        setBody(std::string(_fileaddr, getFileSize()));
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

void
Response::makeResponseForMultipartRange(void) {
    const std::string &path = getRequest()->getResolvedPath();
    RangeList &ranges = getRequest()->getRangeList();

    std::stringstream ss;

    const std::string &sepPrefix = "--";
    const std::string &boundary = "q1w2e3r4";

    RangeList::iterator range = ranges.begin();
    for (range = ranges.begin(); range != ranges.end(); ++range) {
        range->narrow(MAX_RANGE);
        range->rlimit(getFileSize() - 1);

        // Range should not be included if invalid
        ss << sepPrefix << boundary << "\r\n";
        ss << headerNames[CONTENT_TYPE] << getContentType(path) << "\r\n";
        ss << headerNames[CONTENT_RANGE] << getContentRangeValue(*range) << "\r\n";
        ss.write(_fileaddr + range->beg, range->size() - 1);
        ss << "\r\n";
    }
    ss << sepPrefix << boundary << sepPrefix << "\r\n";

    Log.debug() << "Multipart range processed" << Log.endl;

    setStatus(PARTIAL_CONTENT);
    addHeader(CONTENT_TYPE, "multipart/byteranges; boundary=" + boundary);
    setBody(ss.str());    
}

void
Response::makeResponseForRange(void) {

    RangeSet &range = getRequest()->getRangeList()[0];

    // MAX_RANGE is about 2MB now
    range.narrow(MAX_RANGE);
    range.rlimit(getFileSize() - 1);

    Log.debug() << "Range processed: " << range.to_string() << ", " << range.size() << Log.endl;

    setStatus(PARTIAL_CONTENT);
    addHeader(CONTENT_RANGE);
    setBody(std::string(_fileaddr + range.beg, range.size()));
}

int
Response::openFileToResponse(std::string resourcePath) {

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

int
Response::fillFileStat(const std::string &filename, struct stat *st) {

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
    int base = si ? 1000 : 1024;

    if (bytes < base) {
        return lltos(bytes) + suffixes[0] + end;
    }

    int index = static_cast<int>(log(bytes) / log(base));
    double humanSize = static_cast<double>(bytes / pow(base, index));
    char buf[10] = {0};
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

int
Response::listing(const std::string &resourcePath) {

    _body = 
        HTML_BEG HEAD_BEG TITLE_BEG + _req->getPath() + TITLE_END
        META_UTF8 DEFAULT_CSS HEAD_END BODY_BEG
        H1_BEG "Index on " + _req->getPath() + H1_END HR
        TABLE_BEG 
            TR_BEG 
                TH_BEG "Filename" TH_END 
                TH_BEG "Last modified" TH_END
                TH_BEG "Size" TH_END
            TR_END
            TR_BEG 
                TD_BEG "<a href=\"..\">../</a>" TD_END
                TD_BEG TD_END
                TD_BEG TD_END
            TR_END;

    std::deque<std::string> filenames;
    if (!fillDirContent(filenames, resourcePath)) {
        return 0;
    }

    std::sort(filenames.begin(), filenames.end());
    std::deque<std::string>::iterator it;
    for (it = filenames.begin(); it != filenames.end(); ++it) {

        if (*it == "." || *it == "..") {
            continue ;
        }
        _body += createTableLine(*it); 
    }
    _body += TABLE_END HR BODY_END HTML_END;
    setBodyLength(_body.length());

    return 1;
}

int
Response::fillDirContent(std::deque<std::string> &filenames, const std::string &dirName) {

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

    size_t pos = resourcePath.find_last_of('.');
    if (pos != std::string::npos) {
        c_iter it = MIMEs.find(resourcePath.substr(pos + 1));
        if (it != MIMEs.end()) {
            return it->second;
        }
    }
    return "text/plain";
}

void
Response::writeFile(const std::string &resourcePath) {
    std::ofstream outputToNewFile(resourcePath.c_str(),
        std::ios_base::out | std::ios_base::trunc);
    outputToNewFile << _req->getBody();
}

const std::string &
Response::getStatusLine(void) {
    return statusLines[getStatus()];
}

void
Response::makeHead(void) {

    _head.reserve(512);
    _head = getStatusLine();

    for (iter it = headers.begin(); it != headers.end(); ++it) {
        if (it->value.empty()) {
            it->handle(*this);
        }
        if (!it->value.empty()) {
            _head += headerNames[it->hash] + ": " + it->value + "\r\n";
        }
    }

    if (_cgi != NULL) {
        const_iter it = _cgi->getExtraHeaders().begin();
        for ( ; it != _cgi->getExtraHeaders().end(); ++it) {
            _head += it->key + ": " + it->value + "\r\n";
        }
    }

    _head += "\r\n";
}

ResponseHeader *
Response::getHeader(uint32_t hash) {
    std::list<ResponseHeader>::iterator it;
    it = std::find(headers.begin(), headers.end(), ResponseHeader(hash));

    if (it == headers.end()) {
        return NULL;
    } else {
        return &(*it);
    }
}

void
Response::addHeader(uint32_t hash, const std::string &value) {
    ResponseHeader *ptr = getHeader(hash);
    if (ptr == NULL) {
        headers.push_back(ResponseHeader(hash, value));
    } else {
        ptr->value = value;
    }
}

void
Response::addHeader(uint32_t hash) {
    addHeader(hash, "");
}

int
Response::makeResponseForCGI(CGI &cgi) {
    _cgi = &cgi;
    cgi.clear();
    cgi.linkRequest(_req);
    cgi.setEnv();
    if (!cgi.exec()) {
        setStatus(BAD_GATEWAY);
        return 0;
    }
    cgi.parseHeaders();
    setBody(cgi.getBody());

    if (!cgi.isValidContentLength()) {
        setStatus(BAD_GATEWAY);
        return 0;
    }

    const_iter it = cgi.getHeaders().begin();
    const_iter end = cgi.getHeaders().end();
    for (; it != end; ++it) {
        addHeader(it->hash, it->value);
    }

    return 1;
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
//         _res = itoh(_resourceFileStream.gcount()) + "\r\n" + _res + "\r\n";
//     }
//     if (_resourceFileStream.eof()) {
//         _res += "0\r\n\r\n";
//     }
// }

const std::string &
Response::getBody() const {
    return _body;
}

const std::string &
Response::getHead() const {
    return _head;
}

void
Response::setBody(const std::string &body) {
    _body = body;
    setBodyLength(_body.length());
}

size_t
Response::getBodyLength(void) const {
    return _bodyLength;
}

void
Response::setBodyLength(size_t len) {
    _bodyLength = len;
}

Request *
Response::getRequest(void) {
    return _req;
}

StatusCode
Response::getStatus() {
    return _status;
}

void
Response::setStatus(StatusCode status) {
    _status = status;
}

Client *
Response::getClient(void) {
    return getRequest()->getClient();
}

bool
Response::isFormed(void) const {
    return _isFormed;
}

void
Response::isFormed(bool formed) {
    _isFormed = formed;
}

// Not used ?
void *
Response::getFileAddr(void) {
    return _fileaddr;
}

int64_t
Response::getFileSize(void) {
    return _filestat.st_size;
}


std::map<std::string, CGI>::iterator
Response::isCGI(const std::string &filepath) {
    std::map<std::string, CGI> &cgis = _req->getLocation()->getCGIsRef();
    std::map<std::string, CGI>::iterator it  = cgis.begin();
    std::map<std::string, CGI>::iterator end = cgis.end();
    for (; it != end; it++) {
        if (endsWith(filepath, it->first)) {
            it->second.setScriptPath(filepath);
            return it;
        }
    }
    return end;
}

} // namespace HTTP
