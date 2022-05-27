#include "Response.hpp"
#include "CGI.hpp"

HTTP::Response::Response() {
    _responseFormed = false;
}

HTTP::Response::~Response() {}

void    HTTP::Response::clear() {
    _res.clear();
    _resLeftToSend.clear();    
    _responseFormed = false;
}

bool HTTP::Response::isFormed() const {
    return _responseFormed;
}

void HTTP::Response::setFormed(bool formed) {
    _responseFormed = formed;
}

std::string HTTP::Response::GetContentType(std::string resourcePath)
{
    std::string contType;
    for (int i = resourcePath.length(); i--; ) {
        if (resourcePath[i] == '.') {
            std::map<std::string, std::string>::const_iterator iter = 
                _ContType.find(resourcePath.substr(i + 1));
            if (iter != _ContType.end()) {
                    contType = "content-type: " + 
                                (*iter).second;
                    if ((*iter).second == "text") {
                        contType += "; charset=utf-8";
                    }
                    contType += "\r\n";
                }
            break ;
        }
    }
    return (contType);
}

void    HTTP::Response::doCGI(Request &req) {
    std::string resCGI = CGI(req);
     
    if (req.getStatus() >= 400) {
        _res = findErr(req.getStatus());
    } else {
        _res += "Content-Length: ";
        _res += to_string(resCGI.length()) + "\r\n\r\n";
        _res += resCGI;
    }
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

std::string HTTP::Response::fileToResponse(std::string resourcePath) {
    std::ifstream		resourceFile;
    resourceFile.open(resourcePath.c_str(), std::ifstream::in);
    if (!resourceFile.is_open()) {
        _res = findErr(NOT_FOUND);
        return "";
    }

    std::stringstream	buffer;
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

std::string HTTP::Response::listToResponse(std::string &resourcePath, Request &req) {
    std::string pathToDir;
    std::string body =     
        "<!DOCTYPE html>\n"
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
	if (NULL == r_opndir)
	{
        _res = findErr(INTERNAL_SERVER_ERROR);
        return "";
	}
	else
	{
        struct dirent	*r_readdir;
        r_readdir = readdir(r_opndir);
        while (r_readdir) {
            if (NULL == r_readdir) {
                _res = findErr(INTERNAL_SERVER_ERROR);
                return "";
            }

            body += "   <a href=\"" + req.getPath();
            if (body[body.length() - 1] != '/') {
                body += "/";
            }
            body += r_readdir->d_name;
            body += "\">\n<br>";
            body += r_readdir->d_name;
            body += "</a>\n";
		    r_readdir = readdir(r_opndir);
        }
        body += "</body></html>";
	}
	closedir(r_opndir);
    _res += "content-length: " + to_string(body.length()) + "\r\n\r\n";
    return (body);
}

std::string HTTP::Response::resoursePathTaker(Request &req) {
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

std::string HTTP::Response::contentForGetHead(Request &req) {
    // resourcePath (часть root) будет браться из конфига
    // root
    // если в конфиге без /, то добавить /,
    // чтобы мне уже с этим приходило
    std::string resourcePath = resoursePathTaker(req);
    // std::string resourcePath = req.getLocationPtr()->getRootRef();
    // std::cout << req.getLocationPtr()->getIndexRef().empty() << "   ||   " << isFile(resourcePath) << "     resourcePath: " << resourcePath << std::endl;

    _res =
        "HTTP/1.1 200 OK\r\n"
        "connection: keep-alive\r\n"
        "keep-Alive: timeout=55, max=1000\r\n";
    // Path is dir
    if (1 == isFile(resourcePath)) {
        // в дальнейшем заменить на геттер из req
        if (resourcePath[resourcePath.length() - 1] != '/') {
            resourcePath += "/";
        }
        // find index file
        for (std::vector<std::string>::const_iterator iter = req.getLocationPtr()->getIndexRef().begin();
                iter != req.getLocationPtr()->getIndexRef().end() ;
                ++iter) {
            // put index file to response
            if (0 == isFile(resourcePath + *iter)) {
                std::string isCGI = ""; // from config
                if (isCGI != "") {
                    doCGI(req);
                    return (isCGI);
                }
                _res += GetContentType(resourcePath + *iter);
                return (fileToResponse(resourcePath + *iter));
            }
        }
    // Path is file; put Path file to response
    } else if (0 == isFile(resourcePath)) {
        std::string isCGI = ""; // from config
        if (isCGI != "") {
            doCGI(req);
            return (isCGI);
        }
        _res += GetContentType(resourcePath);
        return (fileToResponse(resourcePath));
    }
    // dir listing. autoindex on
    if (req.getLocationPtr()->getAutoindexRef() == true && 
        1 == isFile(resourcePath)) {
        _res += "content-type: text/html; charset=utf-8\r\n";
std::cout << "_res:" + _res << std::endl << std::endl;
        return (listToResponse(resourcePath, req));
    // 403. autoindex off
    } else {
        _res = findErr(FORBIDDEN);
        return "";
    }
}

void HTTP::Response::GETmethod(Request &req) {
    _res += contentForGetHead(req);
}

void HTTP::Response::HEADmethod(Request &req) {
    contentForGetHead(req);
}

void HTTP::Response::DELETEmethod(Request &req) {
    std::string resourcePath = resoursePathTaker(req);
    // чтобы не удалить чистовой сайт я временно добавляю следующую строку:
    resourcePath += "test_empty/1111";

    int isItFile = isFile(resourcePath);
    if (isItFile == -1) {
        _res = findErr(NOT_FOUND);
        return ;
    } else if (isItFile == 2 || std::remove(resourcePath.c_str())) {
        _res = findErr(FORBIDDEN);
        return ;
    }
    _res = 
        "HTTP/1.1 200 OK\r\n"
        "content-length: 60\r\n\r\n"
        "<html>\n"
        "  <body>\n"
        "    <h1>File deleted.</h1>\n"
        "  </body>\n"
        "</html>";
}

void HTTP::Response::POSTmethod(Request &req) {
    std::string isCGI = ""; // from config
    if (isCGI != "") {
        doCGI(req);
    } else {
        _res = "HTTP/1.1 204 No Content\r\n\r\n";
    }
}

const char * HTTP::Response::GetResponse() {
    return (_res.c_str());
}

size_t  HTTP::Response::GetResSize() {
    return (_res.size());
}

const char    *HTTP::Response::GetLeftToSend() {
    return (_resLeftToSend.c_str());
}

void    HTTP::Response::SetLeftToSend(size_t n) {
    _resLeftToSend = _res.substr(n);
}

size_t  HTTP::Response::GetLeftToSendSize() {
    return (_resLeftToSend.size());
}
