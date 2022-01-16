#include "Request.hpp"

Request::Request() : _parseFlags(PARSED_NONE) {}

Request::~Request() {}

// Hide these methods
static void skipSpaces(const std::string &line, size_t &pos) {
	for (; line[pos] == ' '; pos++);
}

static std::string getData(const std::string &line, size_t &pos) {
	size_t tmp = pos;
	size_t end = pos = line.find(' ', pos);
	return line.substr(tmp, end - tmp);
}


int Request::parseStartLine(const std::string &line) {
	
	size_t pos = 0;
	
	_method = getData(line, pos);
	if (!isValidMethod(_method)) {
		return 501;
	}

	skipSpaces(line, pos);
	_path = getData(line, pos);
	if (!isValidPath(_path)) {
		return 404;
	}

	skipSpaces(line, pos);
	_protocol = getData(line, pos);

	skipSpaces(line, pos);
	if (line[pos]) {
		return 400;
	}
	if (!isValidProtocol(_protocol)) {
		return 400;
	}

	setFlag(PARSED_SL);
	return 100;
}


bool Request::isValidMethod(const std::string &method) {
	(void)method;
	return true;
}

bool Request::isValidPath(const std::string &path) {
	(void)path;
	return true;
}

bool Request::isValidProtocol(const std::string &protocol) {
	(void)protocol;
	return true;
}

void Request::setFlag(unsigned char flag) {
	_parseFlags |= flag;
}

void Request::removeFlag(unsigned char flag) {
	_parseFlags &= ~flag;
}

unsigned char Request::getFlags() const {
	return _parseFlags;
}