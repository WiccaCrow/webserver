#include "Response.hpp"
#include "CGI.hpp"

HTTP::Response::Response() {
    _responseFormed = false;
}

HTTP::Response::~Response() { }

void
HTTP::Response::clear() {
    _res.clear();
    _resLeftToSend.clear();
    _responseFormed = false;
}

bool
HTTP::Response::isFormed() const {
    return _responseFormed;
}

void
HTTP::Response::setFormed(bool formed) {
    _responseFormed = formed;
}

std::string
HTTP::Response::GetContentType(std::string resourcePath) {
    std::string contType;
    for (int i = resourcePath.length(); i--;) {
        if (resourcePath[i] == '.') {
            std::map<std::string, std::string>::const_iterator iter = _ContType.find(resourcePath.substr(i + 1));
            if (iter != _ContType.end()) {
                contType = "content-type: " + (*iter).second;
                if ((*iter).second == "text") {
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
HTTP::Response::doCGI(Request &req, CGI &cgi) {
    cgi.reset();
    cgi.setEnv(req);
    if (!cgi.exec()) {
        req.setStatus(HTTP::INTERNAL_SERVER_ERROR);
        _res = findErr(req.getStatus());
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
    std::cout << "file to response:          " << resourcePath << std::endl;
    std::ifstream resourceFile;
    resourceFile.open(resourcePath.c_str(), std::ifstream::in);
    if (!resourceFile.is_open()) {
        _res = findErr(FORBIDDEN);
        return "";
    }

    std::stringstream buffer;
    buffer << resourceFile.rdbuf();
    long bufSize = buffer.tellp();
    if (bufSize == -1) {
        _res = findErr(INTERNAL_SERVER_ERROR);
        return "";
    }
    resourceFile.close();
    // if (bufSize > SIZE_FOR_CHUNKED) {
    //     _res += "Transfer-Encoding: chunked\r\n\r\n";
    //     return (TransferEncodingChunked(buffer.str(), bufSize));
    // } else {
    _res += "content-length: " + to_string(bufSize) + "\r\n\r\n";
    return (buffer.str());
    // }
}

std::string
HTTP::Response::listToResponse(const std::string &resourcePath, Request &req) {
    std::string pathToDir;
    std::string body = "<!DOCTYPE html>\n"
                       "<html>\n"
                       "   <head>\n"
                       "       <meta charset=\"UTF-8\">\n"
                       "       <title> ";
    body += req.getPath() + " </title>\n"
                            "   </head>\n"
                            "<body>\n"
                            "   <h1> Index on ";
    body += req.getPath() + " </h1>\n"
                            "   <p>\n"
                            "   <hr>\n";
    DIR *r_opndir;
    r_opndir = opendir(resourcePath.c_str());
    if (NULL == r_opndir) {
        _res = findErr(INTERNAL_SERVER_ERROR);
        return "";
    } else {
        struct dirent *dirContent;
        // dirContent = readdir(r_opndir);
        while ((dirContent = readdir(r_opndir))) {
            if (NULL == dirContent) {
                _res = findErr(INTERNAL_SERVER_ERROR);
                return "";
            }

            body += "   <a href=\"" + req.getPath();
            if (body[body.length() - 1] != '/') {
                body += "/";
            }
            body += dirContent->d_name;
            body += "\">\n<br>";
            body += dirContent->d_name;
            body += "</a>\n";
            // dirContent = readdir(r_opndir);
        }
        body += "</body></html>";
    }
    closedir(r_opndir);
    _res += "content-length: " + to_string(body.length()) + "\r\n\r\n";
    return (body);
}

std::string
HTTP::Response::resoursePathTaker(Request &req) {
    std::string resourcePath;
    // если URI начинается с /
    resourcePath = req.getLocationPtr()->getRootRef();
    if (req.getPath()[0] == '/') {
    } else {
        resourcePath += "/";
    }

    // root из конфигурации + URI
    resourcePath += req.getPath();
    return (resourcePath);
}

std::map<std::string, HTTP::CGI>::iterator
isCGI(const std::string &filepath, std::map<std::string, HTTP::CGI> &cgi) {
    std::map<std::string, HTTP::CGI>::iterator it  = cgi.begin();
    std::map<std::string, HTTP::CGI>::iterator end = cgi.end();
    for (; it != end; it++) {
        if (endsWith(filepath, it->first)) {
            it->second.setScriptPath(filepath);
            return it;
        }
    }
    return end;
}

std::string
HTTP::Response::contentForGetHead(Request &req) {
    // resourcePath (часть root) будет браться из конфига
    // root
    // если в конфиге без /, то добавить /,
    // чтобы мне уже с этим приходило
    const std::string &resourcePath = req.getPath(); //resoursePathTaker(req);
    // std::string resourcePath = req.getLocationPtr()->getRootRef();
    // std::cout << req.getLocationPtr()->getIndexRef().empty() << "   ||   " << isFile(resourcePath) << "     resourcePath: " << resourcePath << std::endl;

std::cout << "_______     PATH     " << resourcePath << std::endl;

    _res = "HTTP/1.1 200 OK\r\n"
           "connection: keep-alive\r\n"
           "keep-Alive: timeout=55, max=1000\r\n";
    // Path is dir
    if (isDirectory(resourcePath)) {
        // в дальнейшем заменить на геттер из req
        // if (resourcePath[resourcePath.length() - 1] != '/') {
        //     resourcePath += "/";
        // }
        // find index file
        for (std::vector<std::string>::const_iterator iter = req.getLocationPtr()->getIndexRef().begin();
             iter != req.getLocationPtr()->getIndexRef().end();
             ++iter) {
            // put index file to response
            std::string path = resourcePath + *iter;
            if (isFile(path)) {
                std::map<std::string, HTTP::CGI>::iterator it;
                it = isCGI(path, req.getLocationPtr()->getCGIsRef());
                if (it != req.getLocationPtr()->getCGIsRef().end()) {
                    return doCGI(req, it->second);
                }
                _res += GetContentType(path);
                return (fileToResponse(path));
            }
        }
        // Path is file; put Path file to response
    } else if (isFile(resourcePath)) {
        std::map<std::string, CGI>::iterator it;
        it = isCGI(resourcePath, req.getLocationPtr()->getCGIsRef());
        if (it != req.getLocationPtr()->getCGIsRef().end()) {
            return doCGI(req, it->second);
        }
        _res += GetContentType(resourcePath);
        return (fileToResponse(resourcePath));
    } else {
        // not readable files and other types
        _res = findErr(FORBIDDEN);
        return "";
    }
    // dir listing. autoindex on
    if (req.getLocationPtr()->getAutoindexRef() == true && isDirectory(resourcePath)) {
        _res += "content-type: text/html; charset=utf-8\r\n";
        // std::cout << "_res:" + _res << std::endl << std::endl;
        return (listToResponse(resourcePath, req));
        // 403. autoindex off
    } else {
        _res = findErr(FORBIDDEN);
        return "";
    }
}

void
HTTP::Response::GETmethod(Request &req) {
    _res += contentForGetHead(req);
}

void
HTTP::Response::HEADmethod(Request &req) {
    contentForGetHead(req);
}

void
HTTP::Response::DELETEmethod(Request &req) {
    // std::string resourcePath = resoursePathTaker(req);
    // чтобы не удалить чистовой сайт я временно добавляю следующую строку:
    std::string resourcePath = "./testdel";
    (void)req;

    if (!resourceExists(resourcePath)) {
        _res = findErr(NOT_FOUND);
        return;
    } else if (isDirectory(resourcePath)) {
        // deleteDirectory: recursively deletes every file with deleteFile, and call itself for each subdirectory.
        if (rmdirNonEmpty(resourcePath)) {
            _res = findErr(FORBIDDEN);
            return;
        }
    } else {
        if (remove(resourcePath.c_str())) {
            _res = findErr(FORBIDDEN);
            return;
        }
    }
    _res = "HTTP/1.1 200 OK\r\n"
           "content-length: 60\r\n\r\n"
           "<html>\n"
           "  <body>\n"
           "    <h1>File deleted.</h1>\n"
           "  </body>\n"
           "</html>";
}

void
HTTP::Response::POSTmethod(Request &req) {
    std::string isCGI = ""; // from config
    (void)req;
    if (isCGI != "") {
        // doCGI(req);
    } else {
        _res = "HTTP/1.1 204 No Content\r\n\r\n";
    }
}

const char *
HTTP::Response::GetResponse() {
    return (_res.c_str());
}

size_t
HTTP::Response::GetResSize() {
    return (_res.size());
}

const char *
HTTP::Response::GetLeftToSend() {
    return (_resLeftToSend.c_str());
}

void
HTTP::Response::SetLeftToSend(size_t n) {
    _resLeftToSend = _res.substr(n);
}

size_t
HTTP::Response::GetLeftToSendSize() {
    return (_resLeftToSend.size());
}
