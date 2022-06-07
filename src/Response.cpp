#include "Response.hpp"
#include "CGI.hpp"

HTTP::Response::Response() : _shouldBeClosed(false) { }

HTTP::Response::~Response() { }

void
HTTP::Response::initMethodsHeaders(void) {
    methods.insert(std::make_pair("DELETE", &Response::DELETE));
    methods.insert(std::make_pair("HEAD", &Response::HEAD));
    methods.insert(std::make_pair("GET", &Response::GET));
    methods.insert(std::make_pair("OPTIONS", &Response::OPTIONS));
    methods.insert(std::make_pair("POST", &Response::POST));
    methods.insert(std::make_pair("PUT", &Response::PUT));

    headers.insert(std::make_pair(CONTENT_LENGTH, ResponseHeader("content-length", CONTENT_LENGTH)));
    headers.insert(std::make_pair(CONNECTION, ResponseHeader("connection", CONNECTION)));
    headers.insert(std::make_pair(DATE, ResponseHeader("date", DATE)));
    headers.insert(std::make_pair(CONTENT_TYPE, ResponseHeader("content-type", CONTENT_TYPE)));
    // headers.insert(std::make_pair("last-modified", ""));
    // headers.insert(std::make_pair("access-control-allow-methods", ""));
    // headers.insert(std::make_pair("Access-Control-Allow-Origin", ""));
    // headers.insert(std::make_pair("Access-Control-Allow-Credentials", ""));
    // headers.insert(std::make_pair("Access-Control-Expose-Headers", ""));
    // headers.insert(std::make_pair("Access-Control-Max-Age", ""));
    // headers.insert(std::make_pair("Access-Control-Allow-Methods", ""));
    // headers.insert(std::make_pair("Access-Control-Allow-Headers", ""));
}

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

std::string
HTTP::Response::makeHeaders() {

    std::string headersToReturn = statusLines[_req->getStatus()];

    if ((_req->getStatus() >= BAD_REQUEST) ||
        // (если не cgi и методы GET HEAD еще обдумать) ||
        (_req->getMethod() == "PUT" || _req->getMethod() == "DELETE")) {
        headers.find(CONTENT_TYPE)->second.value = "text/html; charset=UTF-8";
    }

    std::map<uint32_t, ResponseHeader>::iterator it    = headers.begin();
    std::map<uint32_t, ResponseHeader>::iterator itEnd = headers.end();
    for (; it != itEnd; ++it) {
        it->second.handleHeader(*this);
        if (!it->second.value.empty()) {
            headersToReturn += it->second.key + ": " + it->second.value + "\r\n";
        }
    }

    headersToReturn += "\r\n";
    return headersToReturn;
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
    contentForGetHead();
}

void
HTTP::Response::GET(void) {
    _res += contentForGetHead();
}

void
HTTP::Response::OPTIONS(void) {
    std::cout << "OPTIONS" << std::endl;
    _res                                    = "HTTP/1.1 200 OK\r\n"
                                              "Allow: ";
    std::vector<std::string> AllowedMethods = _req->getLocation()->getAllowedMethodsRef();
    for (int i = 0, nbMetods = AllowedMethods.size(); i < nbMetods;) {
        _res += AllowedMethods[i];
        if (++i != nbMetods) {
            _res += ", ";
        } else {
            _res += "\r\n\r\n";
        }
    }
}

void
HTTP::Response::POST(void) {
    std::string isCGI = ""; // from config
    if (isCGI != "") {
        // doCGI(*_req);
    } else {
        _res = "HTTP/1.1 204 No Content\r\n\r\n";
    }
}

void
HTTP::Response::PUT(void) {
    std::string resourcePath = _req->getPath();

    if (isDirectory(resourcePath)) {
        setErrorResponse(FORBIDDEN);
    } else if (isFile(resourcePath)) {
        if (isWritableFile(resourcePath)) {
            writeFile(resourcePath);
            _res = "HTTP/1.1 200 OK\r\n"
                   "content-length: 67\r\n\r\n"
                   "<html>\n"
                   "  <body>\n"
                   "    <h1>File is overwritten.</h1>\n"
                   "  </body>\n"
                   "</html>";
        } else {
            setErrorResponse(FORBIDDEN);
        }
    } else {
        writeFile(resourcePath);
        _res = "HTTP/1.1 201 OK\r\n"
               "content-length: 60\r\n\r\n"
               "<html>\n"
               "  <body>\n"
               "    <h1>File created.</h1>\n"
               "  </body>\n"
               "</html>";
    }
}

std::string
HTTP::Response::contentForGetHead(void) {
    // resourcePath (часть root) будет браться из конфига
    // root
    // если в конфиге без /, то добавить /,
    // чтобы мне уже с этим приходило
    const std::string &resourcePath = _req->getResolvedPath();

     if (!resourceExists(resourcePath)) {
        setErrorResponse(NOT_FOUND);
        return "";
    }

    // Should be moved upper
    if (_req->getLocation()->getAuthRef().isSet() && !_req->isAuthorized()) {
        // If no Authorization header provided
        Log.debug("Authenticate");
        _res = statusLines[UNAUTHORIZED];
        _res += "Date: " + getDateTimeGMT() + "\r\n";
        _res += "Connection: close\r\n";
        _res += "WWW-Authenticate: Basic realm=\"" + _req->getLocation()->getAuthRef().getRealmRef() + "\"\r\n";
        _res += "\r\n\r\n";
        shouldBeClosed(true);
        return "";
    }

    _res = "HTTP/1.1 200 OK\r\n"
           "connection: keep-alive\r\n"
           "keep-Alive: timeout=55, max=1000\r\n";
    // Path is dir
    if (isDirectory(resourcePath)) {
        if (resourcePath[resourcePath.length() - 1] != '/') {
            _res = statusLines[MOVED_PERMANENTLY];
            _res += "Location: " + _req->getRawUri() + "/\r\n"
                                                       "Content-Type: text/html\r\n"
                                                       "Connection: keep-alive\r\n\r\n\r\n";
            // body
            return "";
        }
        // find index file
        for (std::vector<std::string>::const_iterator iter = _req->getLocation()->getIndexRef().begin();
             iter != _req->getLocation()->getIndexRef().end();
             ++iter) {
            // put index file to response
            std::string path = resourcePath + *iter;
            if (isFile(path)) {
                std::map<std::string, HTTP::CGI>::iterator it;
                it = isCGI(path, _req->getLocation()->getCGIsRef());
                if (it != _req->getLocation()->getCGIsRef().end()) {
                    return passToCGI(it->second);
                }
                _res += getContentType(path);
                return (fileToResponse(path));
            }
        }
        // Path is file; put Path file to response
    } else if (isFile(resourcePath)) {
        std::map<std::string, CGI>::iterator it;
        it = isCGI(resourcePath, _req->getLocation()->getCGIsRef());
        if (it != _req->getLocation()->getCGIsRef().end()) {
            return passToCGI(it->second);
        }
        _res += getContentType(resourcePath);
        return (fileToResponse(resourcePath));
    } else {
        // not readable files and other types and file not exist
        setErrorResponse(FORBIDDEN);
        return "";
    }
    // dir listing. autoindex on
    if (_req->getLocation()->getAutoindexRef() == true && isDirectory(resourcePath)) {
        _res += "content-type: text/html; charset=utf-8\r\n";
        // std::cout << "_res:" + _res << std::endl << std::endl;
        return (listing(resourcePath));
        // 403. autoindex off
    } else {
        setErrorResponse(FORBIDDEN);
        return "";
    }
}

HTTP::StatusCode
HTTP::Response::handle(Request &req) {
    std::map<std::string, Response::Handler>::iterator it = methods.find(req.getMethod());

    setRequest(&req);
    if (it == methods.end()) {
        return METHOD_NOT_ALLOWED;
    }
    (this->*(it->second))();
    return req.getStatus();
}

void
HTTP::Response::clear() {
    _res.clear();
    _resLeftToSend.clear();
    _body.clear();
}

void
HTTP::Response::setRequest(Request *req) {
    _req = req;
}

HTTP::Request *
HTTP::Response::getRequest(void) const {
    return _req;
}

std::string
HTTP::Response::getContentType(std::string resourcePath) {
    std::string contType;
    for (int i = resourcePath.length() - 1; i >= 0; --i) {
        if (resourcePath[i] == '.') {
            std::map<std::string, std::string>::const_iterator iter = MIMEs.find(resourcePath.substr(i + 1));
            if (iter != MIMEs.end()) {
                contType = "content-type: " + iter->second;
                if (iter->second == "text") {
                    contType += "; charset=utf-8";
                }
                contType += "\r\n";
            }
            break;
        }
    }
    return (contType);
}

std::string
HTTP::Response::passToCGI(CGI &cgi) {
    cgi.reset();
    cgi.linkRequest(_req);
    cgi.setEnv();
    if (!cgi.exec()) {
        _req->setStatus(HTTP::BAD_GATEWAY);
        setErrorResponse(_req->getStatus());

        return "";
    }
    return cgi.getResult();
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

std::string
HTTP::Response::fileToResponse(std::string resourcePath) {
    // std::cout << "file to response:          " << resourcePath << std::endl;
    std::ifstream resourceFile;
    resourceFile.open(resourcePath.c_str(), std::ifstream::in);
    if (!resourceFile.is_open()) {
        setErrorResponse(FORBIDDEN);
        return "";
    }

    std::stringstream buffer;
    buffer << resourceFile.rdbuf();
    long bufSize = buffer.tellp();
    if (bufSize == -1) {
        setErrorResponse(INTERNAL_SERVER_ERROR);
        return "";
    }
    // if (bufSize > SIZE_FOR_CHUNKED) {
    //     _res += "Transfer-Encoding: chunked\r\n\r\n";
    //     return (TransferEncodingChunked(buffer.str(), bufSize));
    // } else {
    _res += "content-length: " + to_string((unsigned long)bufSize) + "\r\n\r\n";
    return (_body = buffer.str());
    // }
}

std::string
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
        setErrorResponse(INTERNAL_SERVER_ERROR);
        return "";
    } else {
        struct dirent *dirContent;
        // dirContent = readdir(r_opndir);
        while ((dirContent = readdir(r_opndir))) {
            if (NULL == dirContent) {
                setErrorResponse(INTERNAL_SERVER_ERROR);
                return "";
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
    _res += "content-length: " + to_string(_body.length()) + "\r\n\r\n";
    return (_body);
}

// Request

// contentForGet
//     makeHeaders
//     makeBody

// contentForHead
//     makeHeaders

void
HTTP::Response::shouldBeClosed(bool flag) {
    _shouldBeClosed = flag;
}

bool 
HTTP::Response::shouldBeClosed(void) const {
    return _shouldBeClosed;
}

void
HTTP::Response::writeFile(const std::string &resourcePath) {
    std::ofstream outputToNewFile(resourcePath.c_str(),
        std::ios_base::out | std::ios_base::trunc); // output file
    outputToNewFile << _req->getBody();               // запись строки в файл
    outputToNewFile.close();
}

const std::string &
HTTP::Response::getBody() const {
    return _body;
}

const char *
HTTP::Response::getResponse() {
    return (_res.c_str());
}

size_t
HTTP::Response::getResLength() {
    return (_res.length());
}

const char *
HTTP::Response::getLeftToSend() {
    return (_resLeftToSend.c_str());
}

void
HTTP::Response::setLeftToSend(size_t n) {
    _resLeftToSend = _res.substr(n);
}

size_t
HTTP::Response::getLeftToSendSize() {
    return (_resLeftToSend.size());
}

void
HTTP::Response::setStatus(HTTP::StatusCode &status) {
    getRequest()->setStatus(status);
}
