#include "Request.hpp"

#include <iostream>

namespace HTTP {

Request::Request() : _parseFlags(PARSED_NONE),
                     _isSizeChunk(true),
                     _sizeChunk(0) {}

Request::~Request() {}

const std::string &Request::getMethod() const {
    return _method;
}

const std::string &Request::getPath() const {
    return _path;
}

const std::string &Request::getProtocol() const {
    return _protocol;
}

const std::map<uint32, Header> &Request::getHeaders() const {
    return _headers;
}

const std::string &Request::getBody() const {
    return _body;
}

const uint8 &Request::getFlags() const {
    return _parseFlags;
}

const HTTP::StatusCode &Request::getStatus() const {
    return _status;
}

bool Request::empty() {
    return (_method.empty() &&
            _path.empty() &&
            _protocol.empty() &&
            _headers.empty());
}

void Request::setFlag(uint8 flag) {
    _parseFlags |= flag;
}

void Request::removeFlag(uint8 flag) {
    _parseFlags &= ~flag;
}

void Request::clear() {
    _method = "";
    // _path.clear();
    _protocol = "";
    _headers.clear();
    _isSizeChunk = true;
    _sizeChunk = 0;
    _body.clear();
    _parseFlags = 0;

    // _method.clear();
    // _path.clear();
    // _protocol.clear();
    // _headers.clear();
    // _isSizeChunk = true;
    // _sizeChunk = 0;
    // _body.clear();
    // _parseFlags = 0;
}

StatusCode Request::parseLine(std::string line) {
    if (!(getFlags() & PARSED_SL)) {
        if ((_status = parseStartLine(line)) != HTTP::CONTINUE) {
            Log.error("Request::parseLine, parsing SL");
            // return HTTP::Response(_status);
            return _status;
        }
    } else if (!(getFlags() & PARSED_HEADERS)) {
        if ((_status = parseHeader(line)) != HTTP::CONTINUE) {
            Log.error("Request::parseLine, parsing Headers");
            // return HTTP::Response(_status);
            return _status;
        }
    } else if (!(getFlags() & PARSED_BODY)) {
        if ((_status = parseBody(line)) != HTTP::CONTINUE) {
            Log.error("Request::parseLine, parsing Body");
            return _status;
        }
    } else {
        return (_status = PROCESSING);
    }
    return CONTINUE;
}

StatusCode Request::parseStartLine(const std::string &line) {
    if (line.empty()) {
        return CONTINUE;
    }
    size_t pos = 0;
    _method = getWord(line, ' ', pos);
    Log.debug("METHOD: " + _method);
    if (isValidMethod(_method) == NOT_IMPLEMENTED) {
        Log.debug("Method is not implemented");
        return NOT_IMPLEMENTED;
    }

    skipSpaces(line, pos);
    _path = getWord(line, ' ', pos);
    Log.debug("PATH: " + _path);
    if (isValidPath(_path) == BAD_REQUEST) {
        Log.debug("Invalid URI");
        return BAD_REQUEST;
    }

    skipSpaces(line, pos);
    _protocol = getWord(line, ' ', pos);

    skipSpaces(line, pos);
    if (line[pos]) {
        std::cout << "|" << (int)line[pos] << "|" << std::endl;
        Log.debug("Forbidden symbols at the end of the line");
        return BAD_REQUEST;
    }
    if (isValidProtocol(_protocol) == HTTP_VERSION_NOT_SUPPORTED) {
        Log.debug("Protocol is not supported or invalid");
        // Or 505, need to improve
        return BAD_REQUEST;
    }

    setFlag(PARSED_SL);
    return CONTINUE;
}

StatusCode Request::isValidMethod(const std::string &method) {
    std::string validMethods[9] = {
        "GET", "DELETE", "POST",
        "PUT", "HEAD", "CONNECT",
        "OPTIONS", "TRACE", "PATCH"};
    for (int i = 0; i < 9; ++i) {
        if (validMethods[i] == method)
            return CONTINUE;
    }
    return NOT_IMPLEMENTED;
}

StatusCode Request::isValidPath(const std::string &path) {
    if (path[0] != '/') {
        return BAD_REQUEST;
    }
    return CONTINUE;
}

StatusCode Request::isValidProtocol(const std::string &protocol) {
    char protocol_valid[] = "HTTP/1.1";
    if (protocol_valid == protocol)
        return CONTINUE;
    return HTTP_VERSION_NOT_SUPPORTED;
}

StatusCode Request::parseHeader(std::string line) {
    if (line == "") {
        setFlag(PARSED_HEADERS);
        Log.debug("Headers were parsed");
        // transfer-encoding && content-length not find
        if (_headers.find(1470906230) == _headers.end() &&
            _headers.find(314322716) == _headers.end()) {
            return PROCESSING;
        }
        return CONTINUE;
    }

    Header header;
    // header.line.swap(line);
    header.line = line;

    size_t sepPos = line.find(':');
    header.line[sepPos] = '\0';
    header.keyLen = sepPos;
    //
    // Some errors skipped with this method... (Maybe should be rewritten with nginx parser)
    header.valStart = line.find_first_not_of(" \t\n\r", sepPos + 1);
    size_t valEnd = line.find_last_not_of(" \t\n\r", sepPos + 1);
    header.valLen = valEnd - header.valStart;

    toLowerCase(header.line);

    header.hash = crc(header.line.data(), header.keyLen);
    std::cout << header.getKey() << std::endl;
    if (static_cast<std::string>(header.getKey()) == static_cast<std::string>("transfer-encoding")) {
        std::cout << header.hash << std::endl;
    }
    if (validHeaders.find(header.hash) == validHeaders.end()) {
        Log.debug("Maybe header is not supported");
        Log.debug(header.line.data() + std::string("    |    ") + to_string(header.hash));
        return BAD_REQUEST;
    }

    // // if host already exists
    // if (header.hash == 3475444733 && _headers.find(header.hash) != _headers.end()) {
    //     return BAD_REQUEST;
    // }

    // dublicate header
    if (_headers.find(header.hash) != _headers.end()) {
        return BAD_REQUEST;
    }
    // Copying here need to replace
    _headers.insert(std::make_pair(header.hash, header));

    return CONTINUE;
}

StatusCode Request::parseChunked(const std::string &line) {
    if (_isSizeChunk) {
        if (line.empty() == true ||
            line.find_first_not_of("0123456789ABCDEFabcdef") != line.npos) {
            // bad chunk length
            _isSizeChunk = false;
            return (BAD_REQUEST);
        }
        std::string chunk(line.c_str());
        if ((_sizeChunk = strtol(chunk.c_str(), NULL, 16)) == 0) {
            if (chunk[0] == '0') {
                setFlag(PARSED_BODY);
                return (PROCESSING);
            }
            return (BAD_REQUEST);
        }
        _isSizeChunk = false;
        return (CONTINUE);
    }

    _isSizeChunk = true;
    if (line.length() > _sizeChunk) {
        // bad chunk body
        return (BAD_REQUEST);
    }
    _sizeChunk = 0;
    _body += line;
    return (CONTINUE);
}

StatusCode Request::parseBody(const std::string &line) {
    // if transfer-encoding
    if (_headers.find(1470906230) != _headers.end()) {
        return (parseChunked(line));
    }
    return PROCESSING;

    // if content-length
    if (_headers.find(314322716) != _headers.end()) {
        long length = atol(_headers[314322716].getVal()); // max content-length?
        // parse
    } else {
        setFlag(PARSED_BODY);
    }
    // return ;
    return CONTINUE;
}

// for chunked
bool Request::getChunked_isSizeChunk() {
    return (_isSizeChunk);
}

void Request::setChunked_isSizeChunk(bool isSize) {
    _isSizeChunk = isSize;
}

long Request::getChunked_Size() {
    return (_sizeChunk);
}
void Request::setChunked_Size(long size) {
    _sizeChunk = size;
}

void Request::setStatus(const HTTP::StatusCode &status) {
    _status = status;
}

}; // namespace HTTP
