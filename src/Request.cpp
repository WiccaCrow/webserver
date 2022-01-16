#include "Request.hpp"

Request::Request(const std::string &line)
{
	parseFirstLine(line);
}

Request::~Request() {}

// Hide these methods
static void skipSpaces(const std::string &line, size_t &pos)
{
	for (; line[pos] == ' '; pos++)
		;
}

static std::string getData(const std::string &line, size_t &pos)
{
	size_t tmp = pos;
	size_t end = pos = line.find(' ', pos);
	return line.substr(tmp, end - tmp);
}

void Request::parseFirstLine(const std::string &line)
{

	size_t pos = 0;

	_method = getData(line, pos);
	skipSpaces(line, pos);
	_path = getData(line, pos);
	skipSpaces(line, pos);
	_protocol = getData(line, pos);
	skipSpaces(line, pos);

	//Validation
	if (line[pos])
		;

	if (!isValidMethod(_method) || !isValidPath(_path) || !isValidProtocol(_protocol))
		;
}

bool Request::isValidMethod(const std::string &method)
{
	char method_valid[9][8] = {
		"GET", "DELETE", "POST"
		// , "PUT", "HEAD", "CONNECT",
		// "OPTIONS", "TRACE", "PATCH"
	};
	for (int i = 0; i < 9; ++i)
	{
		if (method_valid[i] == method)
			return true;
	}
	return false;
	// (void)method;
	// return true;
}

bool Request::isValidPath(const std::string &path)
{
	(void)path;
	return true;
}

bool Request::isValidProtocol(const std::string &protocol)
{
	char protocol_valid[] = "HTTP/1.1";
	if (protocol_valid == protocol)
		return true;
	return false;
	// (void)protocol;
	// return true;
}
