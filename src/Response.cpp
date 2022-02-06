#include "Response.hpp"

HTTP::Response::Response() {
}

const char* HTTP::Response::findErr(int nbErr) {
    std::map<int, const char*>::const_iterator iter = _ErrorCode.find(nbErr);
    return ((*iter).second);
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
        }
    }
    return (contType);
}

// Errors:

// const char *    ErrorCli406(void);
// const char *    ErrorCli409(void);
// const char *    ErrorCli412(void); // ??

// const char *    ErrorServ503(void);

const char* HTTP::Response::GETautoindexOn(std::string resourcePath) {
    _res =
        "HTTP/1.1 200 OK\r\n" + GetContentType(resourcePath);
    _res +=
        "connection: keep-alive\r\n"
        "keep-Alive: timeout=55, max=1000\r\n"
        "content-length: ";

    std::ifstream		resourceFile;
    std::stringstream	buffer;
    resourceFile.open(resourcePath.c_str(), std::ifstream::in);
    if (!resourceFile.is_open()) {
        return (findErr(NOT_FOUND));
    }
    buffer << resourceFile.rdbuf();
    long bufSize = buffer.tellp();
    if (bufSize == -1) {
        return (findErr(INTERNAL_SERVER_ERROR));
    }
    _res += to_string(bufSize) + "\r\n\r\n";
    _res += buffer.str();
    resourceFile.close();
    return _res.c_str();
}

size_t  HTTP::Response::GetResSize() {
    return (_res.size());
}
