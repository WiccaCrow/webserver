#pragma once

#include <map>
#include <string>

class Request {
    private:
    std::string                        _method;
    std::string                        _path;
    std::string                        _protocol;
    std::map<std::string, std::string> _headers;
    std::string                        _body;

    public:
    Request();
    ~Request();

    private:
    void                                 parseFirstLine(const std::string &line);
    bool                                 isValidMethod(const std::string &method);
    bool                                 isValidPath(const std::string &path);
    bool                                 isValidProtocol(const std::string &protocol);
    std::pair<std::string, std::string> &parseHeader(const std::string &line);
};