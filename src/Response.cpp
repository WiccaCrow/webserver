#include "Response.hpp"

HTTP::Response::Response() {
    _responseFormed = false;
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

// void HTTP::Response::TransferEncodingChunked(std::string buffer, size_t bufSize) {
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
// }

void HTTP::Response::fileToResponse(std::string &resourcePath) {
    std::ifstream		resourceFile;
    resourceFile.open(resourcePath.c_str(), std::ifstream::in);
    if (!resourceFile.is_open()) {
        _res = findErr(NOT_FOUND);
        return ;
    }

    std::stringstream	buffer;
    buffer << resourceFile.rdbuf();
    long bufSize = buffer.tellp();
    if (bufSize == -1) {
        _res = findErr(INTERNAL_SERVER_ERROR);
        return ;
    }
    // if (bufSize > SIZE_FOR_CHUNKED) {
    //     _res += "Transfer-Encoding: chunked\r\n\r\n";
    //     TransferEncodingChunked(buffer.str(), bufSize);
    // } else {
    _res += "content-length: " + to_string(bufSize) + "\r\n\r\n";
    _res += buffer.str();
    // }
    resourceFile.close();
}

void HTTP::Response::listToResponse(std::string &resourcePath, Request &req) {
    std::string pathToDir;
    std::string body =     
        "<!DOCTYPE html>\n"
        "<html>\n"
        "   <head>\n"
        "       <meta charset=\"UTF-8\">\n"
        "       <title> " + req.getPath() + " </title>\n"
        "   </head>\n"
        "<body>\n"
        "   <h1> Index of " + req.getPath() + " </h1>\n"
        "   <p>\n"
        "   <hr>\n";
    DIR *r_opndir;
    r_opndir = opendir(resourcePath.c_str());
	if (NULL == r_opndir)
	{
        _res = findErr(INTERNAL_SERVER_ERROR);
        return ;
	}
	else
	{
        struct dirent	*r_readdir;
        r_readdir = readdir(r_opndir);
        while (r_readdir) {
            if (NULL == r_readdir) {
                _res = findErr(INTERNAL_SERVER_ERROR);
                return ;
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
    _res += body;
}

void HTTP::Response::GETmethod(Request &req) {
    // resourcePath (часть root) будет браться из конфига
    // root
    // если в конфиге без /, то добавить /,
    // чтобы мне уже с этим приходило
    std::string resourcePath;
    if (req.getPath()[0] == '/') {
        resourcePath = "./pages/site";
    } else {
        resourcePath += "./pages/site/";
    }
    resourcePath += req.getPath();
    // req.getAutoindex();
    // bool autoindex будет из вышеуказанного геттера
    // bool autoindex = true;
    bool autoindex = false;

    _res =
        "HTTP/1.1 200 OK\r\n";
    if (autoindex == true && 1 == isFile(resourcePath)) {
        if (resourcePath[resourcePath.length() - 1] != '/') {
            resourcePath += "/";
        }
        // перебор файлов из autoindex вектора/массива 
        // до встречи первого совпадения (index.html) или
        // find
        resourcePath += "index.html";
    }
    int isItFile = isFile(resourcePath);
    if (0 == isItFile) {
        _res +=
            GetContentType(resourcePath) +
            "connection: keep-alive\r\n"
            "keep-Alive: timeout=55, max=1000\r\n";
        fileToResponse(resourcePath);
    } else if (autoindex == false && 1 == isItFile) {
        _res +=
            "content-type: text/html; charset=utf-8\r\n"
            "connection: keep-alive\r\n"
            "keep-Alive: timeout=55, max=1000\r\n";
        listToResponse(resourcePath, req);
    } else {
        _res = findErr(NOT_FOUND);
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