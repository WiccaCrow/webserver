#include "Response.hpp"
#include "CGI.hpp"

HTTP::Response::Response() {
//     Methods.reserve(6);
//     Methods[0].first = "DELETE";
//     Methods[1].first = "HEAD";
//     Methods[2].first = "GET";
//     Methods[3].first = "OPTION";
//     Methods[4].first = "POST";
//     Methods[5].first = "PUT";
// 
//     Methods[0].second = Response::&DELETEmethod;
//     Methods[1].second = Response::&HEADmethod;
//     Methods[2].second = Response::&GETmethod;
//     Methods[3].second = Response::&OPTIONSmethod;
//     Methods[4].second = Response::&POSTmethod;
//     Methods[5].second = Response::&PUTmethod;

    Methods.insert(std::make_pair("DELETE", &Response::DELETEmethod));
    Methods.insert(std::make_pair("HEAD", &Response::HEADmethod));
    Methods.insert(std::make_pair("GET", &Response::GETmethod));
    Methods.insert(std::make_pair("OPTION", &Response::OPTIONSmethod));
    Methods.insert(std::make_pair("POST", &Response::POSTmethod));
    Methods.insert(std::make_pair("PUT", &Response::PUTmethod));
}

HTTP::Response::~Response() { }

void
HTTP::Response::clear() {
    _res.clear();
    _resLeftToSend.clear();
}

std::string
HTTP::Response::getContentType(std::string resourcePath) {
    std::string contType;
    for (int i = resourcePath.length(); i--;) {
        if (resourcePath[i] == '.') {
            std::map<std::string, std::string>::const_iterator iter = MIMEs.find(resourcePath.substr(i + 1));
            if (iter != MIMEs.end()) {
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
HTTP::Response::passToCGI(Request &req, CGI &cgi) {
    cgi.reset();
    cgi.setEnv(req);
    if (!cgi.exec()) {
        req.setStatus(HTTP::BAD_GATEWAY);
        setErrorResponse(req.getStatus());

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
    resourceFile.close();
    // if (bufSize > SIZE_FOR_CHUNKED) {
    //     _res += "Transfer-Encoding: chunked\r\n\r\n";
    //     return (TransferEncodingChunked(buffer.str(), bufSize));
    // } else {
    _res += "content-length: " + to_string((unsigned long)bufSize) + "\r\n\r\n";
    return (buffer.str());
    // }
}

std::string
HTTP::Response::listing(const std::string &resourcePath, Request &req) {
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

// Request

// contentForGet
//     makeHeaders
//     makeBody

// contentForHead
//     makeHeaders

std::string
HTTP::Response::contentForGetHead(Request &req) {
    // resourcePath (часть root) будет браться из конфига
    // root
    // если в конфиге без /, то добавить /,
    // чтобы мне уже с этим приходило
    const std::string &resourcePath = req.getResolvedPath();

    _res = "HTTP/1.1 200 OK\r\n"
           "connection: keep-alive\r\n"
           "keep-Alive: timeout=55, max=1000\r\n";
    if (!resourceExists(resourcePath)) {
        setErrorResponse(NOT_FOUND);
        return "";
    }
    // Path is dir
    if (isDirectory(resourcePath)) {
        // в дальнейшем заменить на геттер из req
        if (resourcePath[resourcePath.length() - 1] != '/') {
            _res = statusLines[MOVED_PERMANENTLY];
            _res += "Location: " + req.getRawUri() + "/\r\n"
                                                     "Content-Type: text/html\r\n"
                                                     "Connection: keep-alive\r\n\r\n\r\n";
            // body
            return "";
        }
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
                    return passToCGI(req, it->second);
                }
                _res += getContentType(path);
                return (fileToResponse(path));
            }
        }
        // Path is file; put Path file to response
    } else if (isFile(resourcePath)) {
        std::map<std::string, CGI>::iterator it;
        it = isCGI(resourcePath, req.getLocationPtr()->getCGIsRef());
        if (it != req.getLocationPtr()->getCGIsRef().end()) {
            return passToCGI(req, it->second);
        }
        _res += getContentType(resourcePath);
        return (fileToResponse(resourcePath));
    } else {
        // not readable files and other types and file not exist
        setErrorResponse(FORBIDDEN);
        return "";
    }
    // dir listing. autoindex on
    if (req.getLocationPtr()->getAutoindexRef() == true && isDirectory(resourcePath)) {
        _res += "content-type: text/html; charset=utf-8\r\n";
        // std::cout << "_res:" + _res << std::endl << std::endl;
        return (listing(resourcePath, req));
        // 403. autoindex off
    } else {
        setErrorResponse(FORBIDDEN);
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
    // чтобы не удалить чистовой сайт я временно добавляю следующую строку:
    std::string resourcePath = "./testdel";
    // std::string resourcePath = req.getPath();

    (void)req;

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

void
HTTP::Response::PUTmethod(Request &req) {
    std::string resourcePath = req.getPath();

    if (isDirectory(resourcePath)) {
        setErrorResponse(FORBIDDEN);
    } else if (isFile(resourcePath)) {
        if (isWritableFile(resourcePath)) {
            writeFile(req, resourcePath);
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
        writeFile(req, resourcePath);
        _res = "HTTP/1.1 201 OK\r\n"
               "content-length: 60\r\n\r\n"
               "<html>\n"
               "  <body>\n"
               "    <h1>File created.</h1>\n"
               "  </body>\n"
               "</html>";
    }
}

void
HTTP::Response::OPTIONSmethod(Request &req) {
    std::cout << "OPTIONS" << std::endl;
    _res = "HTTP/1.1 200 OK\r\n"
           "Allow: ";
    std::vector<std::string> AllowedMethods = req.getLocationPtr()->getAllowedMethodsRef();
    for (int i = 0, nbMetods = AllowedMethods.size(); i < nbMetods; ) {
        _res += AllowedMethods[i];
        if (++i != nbMetods) {
            _res += ", ";
        } else {
            _res += "\r\n\r\n";
        }
    std::cout << "_res" << _res << std::endl << std::endl;
    }
}

// void
// HTTP::Response::generateHeaders(Request &req) {
//     
// }


void
HTTP::Response::writeFile(Request &req, const std::string &resourcePath) {
    std::ofstream outputToNewFile(resourcePath.c_str(),
        std::ios_base::out | std::ios_base::trunc); // output file
    outputToNewFile << req.getBody();               // запись строки в файл
    outputToNewFile.close();
}

const char *
HTTP::Response::getResponse() {
    return (_res.c_str());
}

size_t
HTTP::Response::getResSize() {
    return (_res.size());
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
