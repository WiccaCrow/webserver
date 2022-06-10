#include "Response.hpp"
#include "CGI.hpp"
#include "Client.hpp"

std::map<std::string, HTTP::CGI>::iterator
isCGI(const std::string &filepath, std::map<std::string, HTTP::CGI> &cgis) {
    std::map<std::string, HTTP::CGI>::iterator it  = cgis.begin();
    std::map<std::string, HTTP::CGI>::iterator end = cgis.end();
    for (; it != end; it++) {
        if (endsWith(filepath, it->first)) {
            it->second.setScriptPath(filepath);
            return it;
        }
    }
    return end;
}

HTTP::Response::Response() : _req(NULL), _client(NULL) {}

HTTP::Response::~Response() { }

void
HTTP::Response::initMethodsHeaders(void) {
    methods.insert(std::make_pair("DELETE", &Response::DELETE));
    methods.insert(std::make_pair("HEAD", &Response::HEAD));
    methods.insert(std::make_pair("GET", &Response::GET));
    methods.insert(std::make_pair("OPTIONS", &Response::OPTIONS));
    methods.insert(std::make_pair("POST", &Response::POST));
    methods.insert(std::make_pair("PUT", &Response::PUT));

    headers.push_back(ResponseHeader(CONNECTION));
    headers.push_back(ResponseHeader(KEEP_ALIVE));
    headers.push_back(ResponseHeader(DATE));
    headers.push_back(ResponseHeader(SERVER));
    headers.push_back(ResponseHeader(CONTENT_TYPE));
    headers.push_back(ResponseHeader(CONTENT_LENGTH));
}

void
HTTP::Response::clear() {

    _res = "";
    _body = "";
    
    std::list<ResponseHeader>::iterator posIt = headers.begin();
    std::advance(posIt, 6);
    headers.erase(posIt, headers.end());

    std::list<ResponseHeader>::iterator it = headers.begin();
    for (; it != headers.end(); it++) {
        it->value = "";
    }
}

HTTP::StatusCode
HTTP::Response::handle(Request &req) {
    std::map<std::string, Response::Handler>::iterator it = methods.find(req.getMethod());

    if (it == methods.end()) {
        return METHOD_NOT_ALLOWED;
    }
    (this->*(it->second))();
    return req.getStatus();
}

void
HTTP::Response::DELETE(void) {
    // чтобы не удалить чистовой сайт я временно добавляю следующую строку:
    // std::string resourcePath = "./testdel";
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
    _req->setStatus(OK);
    _body = "<html>\n"
            "  <body>\n"
            "    <h1>File deleted.</h1>\n"
            "  </body>\n"
            "</html>";
    _res = makeHeaders() + _body;
}

void
HTTP::Response::HEAD(void) {
    if (contentForGetHead()) {
        _res = makeHeaders();
    } else {
        setErrorResponse(NOT_FOUND);
    }
}

void
HTTP::Response::GET(void) {
    if (contentForGetHead()) {
        _res = makeHeaders() + _body;
    } else {
        std::cout << "GET 1" << std::endl << std::endl;
        setErrorResponse(getStatus());
    }
}

void
HTTP::Response::OPTIONS(void) {
    _res = makeHeaders();
}

void
HTTP::Response::POST(void) {
    std::string isCGI = ""; // from config
    if (!isCGI.empty()) {
        // doCGI(*_req);
    } else {
        _req->setStatus(NO_CONTENT);
        _res = makeHeaders();
    }
}

void
HTTP::Response::PUT(void) {
    std::string resourcePath = _req->getResolvedPath();
    if (isDirectory(resourcePath)) {
        setErrorResponse(FORBIDDEN);
        return ;
    } else if (isFile(resourcePath)) {
        if (isWritableFile(resourcePath)) {
            writeFile(resourcePath);
            _body = "<html>\n"
                    "  <body>\n"
                    "    <h1>File is overwritten.</h1>\n"
                    "  </body>\n"
                    "</html>";
        } else {
            setErrorResponse(FORBIDDEN);
            return ;
        }
    } else {
        writeFile(resourcePath);
        _body = "<html>\n"
                "  <body>\n"
                "    <h1>File created.</h1>\n"
                "  </body>\n"
                "</html>";
    }
    _res = makeHeaders() + _body;
}

int
HTTP::Response::contentForGetHead(void) {

    const std::string &resourcePath = _req->getResolvedPath();

    if (!resourceExists(resourcePath)) {
        setStatus(NOT_FOUND);
        return 0;
    }

    // Should be moved upper
    if (_req->getLocation()->getAuthRef().isSet() && !_req->isAuthorized()) {
        Log.debug("Authenticate:: Unauthorized");
        setStatus(UNAUTHORIZED);
        addHeader(DATE, getDateTimeGMT());
        addHeader(WWW_AUTHENTICATE);
        addHeader(CONNECTION, "close");
        _client->shouldBeClosed(true);
        return 1;
    }

    // Path is dir
    if (isDirectory(resourcePath)) {
        if (resourcePath[resourcePath.length() - 1] != '/') {
            setStatus(MOVED_PERMANENTLY);
            addHeader(LOCATION, _req->getRawUri() + "/");
            addHeader(CONTENT_TYPE, "text/html");
            _body = "<html>\n"
                    "  <body>\n"
                    "    <h1>301 Redirect</h1>\n"
                    "  </body>\n"
                    "</html>\r\n";
            return 1;
        }
        // find index file
        const std::vector<std::string> &indexes = _req->getLocation()->getIndexRef();
        for (size_t i = 0; i < indexes.size(); ++i) {
            // put index file to response
            std::string path = resourcePath + indexes[i];
            if (isFile(path)) {
                std::map<std::string, HTTP::CGI>::iterator it;
                it = isCGI(path, _req->getLocation()->getCGIsRef());
                if (it != _req->getLocation()->getCGIsRef().end()) {
                    return passToCGI(it->second);
                }
                addHeader(CONTENT_TYPE, getContentType(path));
                addHeader(ETAG, getEtagFile(path));
                addHeader(LAST_MODIFIED, getLastModifiedTimeGMT(path));
                return fileToResponse(path);
            }
        }
        // Path is file; put Path file to response
    } else if (isFile(resourcePath)) {
        std::map<std::string, CGI>::iterator it;
        it = isCGI(resourcePath, _req->getLocation()->getCGIsRef());
        if (it != _req->getLocation()->getCGIsRef().end()) {
            return passToCGI(it->second);
        }
        addHeader(CONTENT_TYPE, getContentType(resourcePath));
        addHeader(ETAG, getEtagFile(resourcePath));
        addHeader(LAST_MODIFIED, getLastModifiedTimeGMT(resourcePath));
        return fileToResponse(resourcePath);
    } else {
        // not readable files and other types and file not exist
        setStatus(FORBIDDEN);
        return 0;
    }
    // dir listing. autoindex on
    if (_req->getLocation()->getAutoindexRef() == true && isDirectory(resourcePath)) {
        addHeader(CONTENT_TYPE, "text/html; charset=utf-8");
        return listing(resourcePath);
        // 403. autoindex off
    } else {
        setStatus(FORBIDDEN);
        return 0;
    }
}

int
HTTP::Response::fileToResponse(std::string resourcePath) {
    std::ifstream resourceFile;
    resourceFile.open(resourcePath.c_str(), std::ifstream::in);
    if (!resourceFile.is_open()) {
        setStatus(FORBIDDEN);
        return 0;
    }

    std::stringstream buffer;
    buffer << resourceFile.rdbuf();
    if (buffer.tellp() == -1) {
        setStatus(INTERNAL_SERVER_ERROR);
        return 0;
    }

    // if (bufSize > SIZE_FOR_CHUNKED) {
    //     _res += "Transfer-Encoding: chunked\r\n\r\n";
    //     return (TransferEncodingChunked(buffer.str(), bufSize));
    // }

    _body = buffer.str();
    return 1;
}

int
HTTP::Response::listing(const std::string &resourcePath) {
    std::string pathToDir;
    _body = "<!DOCTYPE html>\n"
            "<html>\n"
            "   <head>\n"
            "       <meta charset=\"UTF-8\">\n"
            "       <title> ";
    _body += _req->getPath() + " </title>\n"
                               "   </head>\n"
                               "<body>\n"
                               "   <h1> Index on ";
    _body += _req->getPath() + " </h1>\n"
                               "   <p>\n"
                               "   <hr>\n";
    DIR *r_opndir;
    r_opndir = opendir(resourcePath.c_str());
    if (NULL == r_opndir) {
        setStatus(INTERNAL_SERVER_ERROR);
        return 0;
    } else {
        struct dirent *dirContent;
        // dirContent = readdir(r_opndir);
        while ((dirContent = readdir(r_opndir))) {
            if (NULL == dirContent) {
                setStatus(INTERNAL_SERVER_ERROR);
                return 0;
            }

            _body += "   <a href=\"" + _req->getPath();
            if (_body[_body.length() - 1] != '/') {
                _body += "/";
            }
            _body += dirContent->d_name;
            _body += "\">\n<br>";
            _body += dirContent->d_name;
            _body += "</a>\n";
            // dirContent = readdir(r_opndir);
        }
        _body += "</body></html>";
    }
    closedir(r_opndir);
    return 1;
}

std::string
HTTP::Response::getContentType(std::string resourcePath) {
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
HTTP::Response::writeFile(const std::string &resourcePath) {
    std::ofstream outputToNewFile(resourcePath.c_str(),
        std::ios_base::out | std::ios_base::trunc);
    outputToNewFile << _req->getBody();
}

std::string
HTTP::Response::makeHeaders() {

    std::string headersToReturn = statusLines[_req->getStatus()];

    if ((_req->getStatus() >= BAD_REQUEST) ||
        // (если не cgi и методы GET HEAD еще обдумать) ||
        (_req->getMethod() == "PUT" || _req->getMethod() == "DELETE")) {
        addHeader(CONTENT_TYPE, "text/html; charset=UTF-8");
    }

    std::list<ResponseHeader>::iterator it    = headers.begin();
    std::list<ResponseHeader>::iterator itEnd = headers.end();
    for (; it != itEnd; ++it) {
        it->handle(*this);
        if (!it->value.empty()) {
            headersToReturn += headerNames[it->hash] + ": " + it->value + "\r\n";
        }
    }

    headersToReturn += _additionalHeaders;
    headersToReturn += "\r\n";
    return headersToReturn;
}

HTTP::ResponseHeader *
HTTP::Response::getHeader(HeaderCode code) {
    std::list<ResponseHeader>::iterator it;
    it = std::find(headers.begin(), headers.end(), ResponseHeader(code));

    if (it == headers.end()) {
        return NULL;
    } else {
        return &(*it);
    }
}

void
HTTP::Response::addHeader(HeaderCode code, const std::string &value) {
    ResponseHeader *ptr = getHeader(code);
    if (ptr == NULL) {
        headers.push_back(ResponseHeader(code, value));
    } else {
        ptr->value = value;
    }
}

void
HTTP::Response::addHeader(HeaderCode code) {
    addHeader(code, "");
}

int
HTTP::Response::passToCGI(CGI &cgi) {
    cgi.reset();
    cgi.linkRequest(_req);
    cgi.setEnv();
    if (!cgi.exec()) {
        setStatus(HTTP::BAD_GATEWAY);
        return 0;
    }
    _body = cgi.getResult();
    return 1;
}

size_t
HTTP::Response::getResLength() {
    return (_res.length());
}

const char *
HTTP::Response::getResponse() {
    return (_res.c_str());
}

const std::string &
HTTP::Response::getBody() const {
    return _body;
}

void
HTTP::Response::setRequest(Request *req) {
    _req = req;
}

HTTP::Request *
HTTP::Response::getRequest(void) const {
    return _req;
}

HTTP::StatusCode
HTTP::Response::getStatus() {
    return getRequest()->getStatus();
}

void
HTTP::Response::setStatus(HTTP::StatusCode status) {
    getRequest()->setStatus(status);
}

void
HTTP::Response::setClient(HTTP::Client *client) {
    _client = client;
}

HTTP::Client *
HTTP::Response::getClient(void) {
    return _client;
}

// std::string HTTP::Response::TransferEncodingChunked(std::string buffer, size_t bufSize) {
//     size_t i = 0;
//     std::string sizeChunck = itoh(SIZE_FOR_CHUNKED) + "\r\n";
//     while (bufSize > i + SIZE_FOR_CHUNKED) {
//         _res += sizeChunck;
//         _res += buffer.substr(i, (size_t)SIZE_FOR_CHUNKED) + "\r\n";
//         i += SIZE_FOR_CHUNKED;
//     }
//     _res += itoh(bufSize - i) + "\r\n";
//     _res += buffer.substr(i, bufSize - i) + "\r\n"
//             "0\r\n\r\n";
//     return ("");
// }
