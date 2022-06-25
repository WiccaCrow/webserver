#include "Response.hpp"
#include "CGI.hpp"
#include "Client.hpp"
#include "Server.hpp"

namespace HTTP {

Response::Response() 
    : _req(NULL)
    , _client(NULL)
    , _cgi(NULL)
    , _bodyLength(0)
    , _isFormed(false) {}

Response::Response(Request *req) 
    : _req(req)
    , _client(NULL)
    , _cgi(NULL)
    , _bodyLength(0)
    , _isFormed(false)
    , _status(req->getStatus()) {
    methods.insert(std::make_pair("GET", &Response::GET));
    methods.insert(std::make_pair("PUT", &Response::PUT));
    methods.insert(std::make_pair("POST", &Response::POST));
    methods.insert(std::make_pair("HEAD", &Response::HEAD));
    methods.insert(std::make_pair("DELETE", &Response::DELETE));
    methods.insert(std::make_pair("OPTIONS", &Response::OPTIONS));
    methods.insert(std::make_pair("CONNECT", &Response::CONNECT));

    headers.push_back(ResponseHeader(DATE));
    headers.push_back(ResponseHeader(SERVER));
    headers.push_back(ResponseHeader(KEEP_ALIVE));
    headers.push_back(ResponseHeader(CONNECTION));
    headers.push_back(ResponseHeader(CONTENT_TYPE));
    headers.push_back(ResponseHeader(CONTENT_LENGTH));
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
        methods = other.methods;
    }
    return *this;
}

Response::~Response() { 
    if (getRequest() != NULL) {
        delete getRequest();
    } 
}

void
Response::initMethodsHeaders(void) {}

void
Response::clear() {}

void
Response::handle(void) {
    std::map<std::string, Response::Handler>::iterator it;
                std::cout << "         test 2  " << std::endl;

    if (getStatus() >= BAD_REQUEST) {
        this->setErrorResponse(getStatus());
    } else if (_req->getMethod() == "") {
        _res = _body = _req->getBody();
        isFormed(true);
                std::cout << "         test 3  "<< std::endl;
        return ;
    } else if (_req->authNeeded() && !_req->isAuthorized()) {
        this->unauthorized();
    } else {
        it = methods.find(_req->getMethod());
        (this->*(it->second))();
    }

    _res = getStatusLine() + makeHeaders() + getBody();
                std::cout << "         test 4  "<< std::endl;
    isFormed(true);
}

void
Response::unauthorized(void) {
    Log.debug() << "Response:: Unauthorized" << std::endl;
    setStatus(UNAUTHORIZED);
    addHeader(DATE, Time::gmt());
    addHeader(WWW_AUTHENTICATE);
    getClient()->shouldBeClosed(true);
}

// void
// Response::proxyRun() {
//     _client->setProxyUri(&_req->getUriRef());
//     size_t id = _client->proxyRun();
//     _status   = _client->getProxyStatus();

//     if (_status == OK) {
//         setBody("Connection Established.");
//         g_server->getClient(id)->setProxyFdOut(_client->getFd());
//         g_server->getClient(id)->setProxyidOtherSide(_client->getId());
//     }
// }

void
Response::makeProxyResponse(std::string response) {
    _body = response;
    _res = _body;
    isFormed(true);
}

void
Response::CONNECT(void) {
    setStatus(OK);
    setBody("Connection Established.");
}

void
Response::DELETE(void) {
    std::string resourcePath = _req->getResolvedPath();

    if (!resourceExists(resourcePath)) {
        setErrorResponse(NOT_FOUND);
        return;
    } else if (isDirectory(resourcePath)) {
        if (rmdirNonEmpty(resourcePath)) {
            setErrorResponse(FORBIDDEN);
            return;
        }
    } else {
        if (remove(resourcePath.c_str())) {
            setErrorResponse(FORBIDDEN);
            return;
        }
    }
    setStatus(OK);
    setBody("<html><body>"
            " <h1>File deleted.</h1>"
            "</body></html>");
}

void
Response::HEAD(void) {
    if (!contentForGetHead()) {
        setErrorResponse(getStatus());
    } else {
        _body = "";
    }
}

void
Response::GET(void) {
    if (!contentForGetHead()) {
        setErrorResponse(getStatus());
    }
    addHeader(ACCEPT_RANGES, "none");
}

void
Response::OPTIONS(void) {
    addHeader(ALLOW);
}

void
Response::POST(void) {
    std::map<std::string, CGI>::iterator it;
    it = isCGI(_req->getResolvedPath(), _req->getLocation()->getCGIsRef());
    if (it != _req->getLocation()->getCGIsRef().end()) {
        passToCGI(it->second);
    } else {
        setStatus(NO_CONTENT);
    }
}

void
Response::PUT(void) {
    std::string resourcePath = _req->getResolvedPath();
    if (isDirectory(resourcePath)) {
        setErrorResponse(FORBIDDEN);
        return ;
    } else if (isFile(resourcePath)) {
        if (isWritableFile(resourcePath)) {
            writeFile(resourcePath);
            setBody("<html>"
                    "<body>"
                    " <h1>File is overwritten.</h1>"
                    "</body>"
                    "</html>");
        } else {
            setErrorResponse(FORBIDDEN);
            return ;
        }
    } else {
        writeFile(resourcePath);
        setBody("<html>"
                "<body>"
                " <h1>File created.</h1>"
                "</body>"
                "</html>");
    }
}

int
Response::contentForGetHead(void) {
    std::string resourcePath = _req->getResolvedPath();

    if (!resourceExists(resourcePath)) {
        setStatus(NOT_FOUND);
        return 0;
    }

    if (isDirectory(resourcePath)) {
        if (redirectForDirectory(resourcePath)) {
            return 1;
        } else if (isSetIndexFile(resourcePath)) {
            return makeGetHeadResponseForFile(resourcePath);
        } else {
            return directoryListing(resourcePath);
        }
    } else if (isFile(resourcePath)) {
        return makeGetHeadResponseForFile(resourcePath);
    } else {
        // not readable files and other types
        setStatus(FORBIDDEN);
        return 0;
    }
}

int
Response::redirectForDirectory(const std::string &resourcePath) {
    if (resourcePath[resourcePath.length() - 1] != '/') {
        setStatus(MOVED_PERMANENTLY);
        addHeader(LOCATION, _req->getRawUri() + "/");
        addHeader(CONTENT_TYPE, "text/html");
        setBody("<html><body>"
                " <h1>Redirect 301</h1>"
                "</body></html>");
        return 1;
    }
    return 0;
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
Response::makeGetHeadResponseForFile(const std::string &resourcePath) {
    std::map<std::string, CGI>::iterator it;
    it = isCGI(resourcePath, _req->getLocation()->getCGIsRef());
    if (it != _req->getLocation()->getCGIsRef().end()) {
        return passToCGI(it->second);
    }
    addHeader(CONTENT_TYPE, getContentType(resourcePath));
    addHeader(ETAG, getEtagFile(resourcePath));
    addHeader(LAST_MODIFIED, Time::gmt(getModifiedTime(resourcePath)));
    addHeader(TRANSFER_ENCODING, "chunked");
    return openFileToResponse(resourcePath);
}

int
Response::directoryListing(const std::string &resourcePath) {
    // autoindex on
    if (_req->getLocation()->getAutoindexRef() && isDirectory(resourcePath)) {
        addHeader(CONTENT_TYPE, "text/html; charset=utf-8");
        return listing(resourcePath);
    } else {
        // autoindex off
        setStatus(FORBIDDEN);
        return 0;
    }
}

int
Response::openFileToResponse(std::string resourcePath) {
    _resourceFileStream.open(resourcePath.c_str(), std::ifstream::in | std::ios_base::binary);
    if (!_resourceFileStream.is_open()) {
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
        // Need to improve, because utf could contain more than 2 bytes
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
        Log.debug() << "cannot get stat of " << fullname << std::endl;
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
Response::getContentType(std::string resourcePath) {
    std::string contType;
    for (int i = resourcePath.length() - 1; i >= 0; --i) {
        if (resourcePath[i] == '.') {
            std::map<std::string, std::string>::const_iterator iter = MIMEs.find(resourcePath.substr(i + 1));
            if (iter != MIMEs.end()) {
                contType = iter->second;
                if (iter->second == "text") {
                    contType += "; charset=utf-8";
                }
            }
            break;
        }
    }
    return (contType);
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

std::string
Response::makeHeaders() {

    std::string allHeaders;

    if ((getStatus() >= BAD_REQUEST) ||
        // (если не cgi и методы GET HEAD еще обдумать) ||
        (_req->getMethod() == "PUT" || _req->getMethod() == "DELETE")) {
        addHeader(CONTENT_TYPE, "text/html; charset=UTF-8");
    }

    for (iter it = headers.begin(); it != headers.end(); ++it) {
        if (it->value.empty()) {
            it->handle(*this);
        }
        if (!it->value.empty()) {
            allHeaders += headerNames[it->hash] + ": " + it->value + "\r\n";
        }
    }

    if (_cgi != NULL) {
        const_iter it = _cgi->getExtraHeaders().begin();
        for ( ; it != _cgi->getExtraHeaders().end(); ++it) {
            allHeaders += it->key + ": " + it->value + "\r\n";
        }
    }

    allHeaders += "\r\n";

    // Log.debug() << std::endl << allHeaders << std::endl;
    return allHeaders;
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
Response::passToCGI(CGI &cgi) {
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

void
Response::makeChunk() {
    _res = "";

    if (!getHeader(TRANSFER_ENCODING) ) {
        return ;
    }

    if (_resourceFileStream.eof()) {
        _resourceFileStream.close();
        _resourceFileStream.clear();
        return ;
    }
    char buffer[CHUNK_SIZE] = {0};
    _resourceFileStream.read(buffer, sizeof(buffer) - 1);
    if (_resourceFileStream.fail() && !_resourceFileStream.eof()) {
        setStatus(INTERNAL_SERVER_ERROR);
        _client->shouldBeClosed(true);
        return ;
    } else {
        _res.assign(buffer, _resourceFileStream.gcount());
        _res = itoh(_resourceFileStream.gcount()) + "\r\n" + _res + "\r\n";
    }
    if (_resourceFileStream.eof()) {
        _res += "0\r\n\r\n";
    }
}

size_t
Response::getResponseLength() {
    return (_res.length());
}

const std::string &
Response::getResponse() {
    return _res;
}

const std::string &
Response::getBody() const {
    return _body;
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

std::map<std::string, CGI>::iterator
Response::isCGI(const std::string &filepath, std::map<std::string, CGI> &cgis) {
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
