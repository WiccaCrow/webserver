#include "Response.hpp"

HTTP::Response::Response() {
    _responseFormed = false;
}

void    HTTP::Response::clear() {
    _res.clear();
    _resLeftToSend.clear();    
    _responseFormed = false;
}

const char* HTTP::Response::findErr(int nbErr) {
    std::map<int, const char*>::const_iterator iter = _ErrorCode.find(nbErr);
    _res = (*iter).second;
    return (_res.c_str());
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

std::string         HTTP::Response::doCGI(Request &req) {
    // две строки ниже убрать, когда будет использован в коде req
    if (req.getMethod() != "")
        ;
    std::string body;
    // раскомментировать, когда  будет готов глобальный объект для конфига
    // добавить в объект конфига функцию, которая принимает строку с URI req.getPath(),
    // которую будет сравнивать с map для cgi, например такой: map<окончание для url, путь к cgi> cgiMap;
    // и возвращает std::string путь к cgi для принятия функцией
    // executeCGI(необходимые параметры).
    //      std::string путь к cgi = объект конфига  conf.cmpCGI(req.getPath());
    //      if (путь к cgi != "") {
    //          std::string resCGI = executeCGI(необходимые параметры);
    //
    // _req.getStatus() еще не написано, но уже обговорено.
    // это будет либо status в pubic у Request, либо геттер на него
    // Используется также в reply  в Client.cpp
    //      Внутри executeCGI(...) в случае ошибки, необходимо изменить значение 
    //      этого кода, например написать для этого SetStatus(int statusCode) в Request
    //          int _req_getStatus = HTTP::OK;
    //          if (_req_getStatus >= 400) {
    //              _res = "";
    //              return (findErr(_req_getStatus));
    //          }
    //              
    //          _res += "content length: ";
    //          _res += to_string(resCGI.length()) + "\r\n\r\n";
    //          body = resCGI;
    //      }
    return (body);
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

std::string HTTP::Response::fileToResponse(std::string &resourcePath) {
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
        "   <h1> Index of ";
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
    if (req.getPath()[0] == '/') {
        resourcePath = "./pages/site";
    } else {
        resourcePath += "./pages/site/";
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
    // req.getAutoindex();
    // bool autoindex будет из вышеуказанного геттера
    // bool autoindex = true;
    bool autoindex = false;

    _res =
        "HTTP/1.1 200 OK\r\n"
        "connection: keep-alive\r\n"
        "keep-Alive: timeout=55, max=1000\r\n";
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
        std::string isCGI = doCGI(req);
        if (isCGI != "") {
            return (isCGI);
        }
        _res += GetContentType(resourcePath);
        return (fileToResponse(resourcePath));
    } else if (autoindex == false && 1 == isItFile) {
        _res += "content-type: text/html; charset=utf-8\r\n";
std::cout << "_res:" + _res << std::endl << std::endl;
        return (listToResponse(resourcePath, req));
    } else {
        _res = findErr(NOT_FOUND);
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
    std::string isCGI = doCGI(req);
    if (isCGI == "") {
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