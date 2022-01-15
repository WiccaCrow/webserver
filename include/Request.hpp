#pragma once

#include <string>
#include <map>

class Request {

    std::string                         _method;
    std::string                         _path;
    std::string                         _protocol;
    std::map<std::string, std::string>  _headers;
    std::string                         _body;
    
	public:
	Request(const std::string &line);
	~Request();
	
	private:
	void parseFirstLine(const std::string &line);
	bool isValidMethod(const std::string &method);
	bool isValidPath(const std::string &path);
	bool isValidProtocol(const std::string &protocol);
    std::pair<std::string, std::string> &parseHeader(const std::string &line);

};