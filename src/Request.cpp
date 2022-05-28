#include "Request.hpp"

namespace HTTP
{

    Request::Request(ServerBlock &servBlock) : _servBlock(servBlock),
                                               _location_current(NULL),
                                               _flag_getline_bodySize(true),
                                               _bodySize(0),
                                               _parseFlags(PARSED_NONE)
    {
        // временная мера test cout
        _location_current = &((servBlock.getLocationsRef().find("/about"))->second);
    }

    Request::~Request() {}

    Request::Request(const Request &other) : _servBlock(other._servBlock)
    {
        *this = other;
    }

    Request &Request::operator=(const Request &other)
    {
        if (this != &other)
        {
            _method = other._method;
            _path = other._path;
            _protocol = other._protocol;
            _queryString = other._queryString;
            _scriptName = other._scriptName;
            _headers = other._headers;
            _status = other._status;
            _servBlock = other._servBlock;
            _flag_getline_bodySize = other._flag_getline_bodySize;
            _bodySize = other._bodySize;
            _body = other._body;
            _parseFlags = other._parseFlags;
        }
        return *this;
    }

    const ServerBlock &Request::getServerBlock() const
    {
        return _servBlock;
    }

    const std::string &Request::getMethod() const
    {
        return _method;
    }

    const std::string &Request::getPath() const
    {
        return _path;
    }

    const std::string &Request::getProtocol() const
    {
        return _protocol;
    }

    const std::map<HeaderCode, Header> &Request::getHeaders() const
    {
        return _headers;
    }

    const std::string &Request::getBody() const
    {
        return _body;
    }

    const uint8_t &Request::getFlags() const
    {
        return _parseFlags;
    }

    const HTTP::StatusCode &Request::getStatus() const
    {
        return _status;
    }

    Location *Request::getLocationPtr()
    {
        return _location_current;
    }

    bool Request::empty()
    {
        return (_method.empty() &&
                _path.empty() &&
                _protocol.empty() &&
                _headers.empty());
    }

    void Request::setFlag(uint8_t flag)
    {
        _parseFlags |= flag;
    }

    void Request::removeFlag(uint8_t flag)
    {
        _parseFlags &= ~flag;
    }

    void Request::clear()
    {
        _method = "";
        // _path.clear();
        _protocol = "";
        _headers.clear();
        _flag_getline_bodySize = true;
        _bodySize = 0;
        _body.clear();
        _parseFlags = 0;

        // _method.clear();
        // _path.clear();
        // _protocol.clear();
        // _headers.clear();
        // _flag_getline_bodySize = true;
        // _bodySize = 0;
        // _body.clear();
        // _parseFlags = 0;
    }

    const std::string &Request::getQueryString() const
    {
        return _queryString;
    }

    const std::string &Request::getScriptName() const
    {
        return _scriptName;
    }

    const char *Request::getHeaderValue(HeaderCode key) const
    {
        std::map<HeaderCode, Header>::const_iterator it = _headers.find(key);
        if (it == _headers.end())
        {
            return "";
        }
        return it->second.getVal();
    }

    // scheme://authority/path/?query_string#fragment

    StatusCode Request::parseLine(std::string line)
    {
        if (!(getFlags() & PARSED_SL))
        {
            if ((_status = parseStartLine(line)) != HTTP::CONTINUE)
            {
                Log.error("Request::parseLine, parsing SL");
                // return HTTP::Response(_status);
                return _status;
            }
        }
        else if (!(getFlags() & PARSED_HEADERS))
        {
            if ((_status = parseHeader(line)) != HTTP::CONTINUE)
            {
                Log.error("Request::parseLine, parsing Headers");
                // return HTTP::Response(_status);
                return _status;
            }
        }
        else if (!(getFlags() & PARSED_BODY))
        {
            if ((_status = parseBody(line)) != HTTP::CONTINUE)
            {
                Log.error("Request::parseLine, parsing Body");
                return _status;
            }
        }
        else
        {
            return (_status = PROCESSING);
        }
        return CONTINUE;
    }

    StatusCode Request::parseStartLine(const std::string &line)
    {
        if (line.empty())
        {
            return CONTINUE;
        }
        size_t pos = 0;
        _method = getWord(line, ' ', pos);
        Log.debug("METHOD: " + _method);
        if (isValidMethod(_method) == NOT_IMPLEMENTED)
        {
            Log.debug("Method is not implemented");
            return NOT_IMPLEMENTED;
        }

        skipSpaces(line, pos);
        _path = getWord(line, ' ', pos);
        Log.debug("PATH: " + _path);
        if (isValidPath(_path) == BAD_REQUEST)
        {
            Log.debug("Invalid URI");
            return BAD_REQUEST;
        }

        skipSpaces(line, pos);
        _protocol = getWord(line, ' ', pos);

        skipSpaces(line, pos);
        if (line[pos])
        {
            // std::cout << "|" << (int)line[pos] << "|" << std::endl;
            Log.debug("Forbidden symbols at the end of the line");
            return BAD_REQUEST;
        }
        if (isValidProtocol(_protocol) == HTTP_VERSION_NOT_SUPPORTED)
        {
            Log.debug("Protocol is not supported or invalid");
            // Or 505, need to improve
            return BAD_REQUEST;
        }

        setFlag(PARSED_SL);
        return CONTINUE;
    }

    StatusCode Request::isValidMethod(const std::string &method)
    {
        std::string validMethods[9] = {
            "GET", "DELETE", "POST",
            "PUT", "HEAD", "CONNECT",
            "OPTIONS", "TRACE", "PATCH"};
        for (int i = 0; i < 9; ++i)
        {
            if (validMethods[i] == method)
                return CONTINUE;
        }
        return NOT_IMPLEMENTED;
    }

    StatusCode Request::isValidPath(const std::string &path)
    {
        if (path[0] != '/')
        {
            return BAD_REQUEST;
        }
        return CONTINUE;
    }

    StatusCode Request::isValidProtocol(const std::string &protocol)
    {
        char protocol_valid[] = "HTTP/1.1";
        if (protocol_valid == protocol)
            return CONTINUE;
        return HTTP_VERSION_NOT_SUPPORTED;
    }

    StatusCode Request::parseHeader(std::string line)
    {
        if (line == "")
        {
            setFlag(PARSED_HEADERS);
            Log.debug("Headers were parsed");
            // transfer-encoding && content-length not find
            if (_headers.find(TRANSFER_ENCODING) == _headers.end() &&
                _headers.find(CONTENT_LENGTH) == _headers.end())
            {
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

        header.hash = static_cast<HeaderCode>(crc(header.line.data(), header.keyLen));
        std::map<uint32_t, Header::Handler>::const_iterator it = validHeaders.find(header.hash);
        header.handler = it->second;
        if (it == validHeaders.end())
        {
            Log.debug("Maybe header is not supported");
            Log.debug(header.line.data() + std::string("    |    ") + to_string(header.hash));
            return BAD_REQUEST;
        }

        // dublicate header
        if (_headers.find(header.hash) != _headers.end())
        {
            return BAD_REQUEST;
        }
        if (header.hash == CONTENT_LENGTH)
        {
            size_t length = strtoul(header.getVal(), NULL, 10);
            setBodySizeFlag(false);
            setBodySize(length);
        }

        // Copying here need to replace
        _headers.insert(std::make_pair(header.hash, header));

        return CONTINUE;
    }

    StatusCode Request::parseChunked(const std::string &line)
    {
        if (_flag_getline_bodySize)
        {
            if (line.empty() == true ||
                line.find_first_not_of("0123456789ABCDEFabcdef") != line.npos)
            {
                // bad chunk length
                _flag_getline_bodySize = false;
                return (BAD_REQUEST);
            }
            std::string chunk(line.c_str());
            if ((_bodySize = strtoul(chunk.c_str(), NULL, 16)) == 0)
            {
                if (chunk[0] == '0')
                {
                    setFlag(PARSED_BODY);
                    return (PROCESSING);
                }
                return (BAD_REQUEST);
            }
            _flag_getline_bodySize = false;
            return (CONTINUE);
        }

        _flag_getline_bodySize = true;
        if (line.length() > _bodySize)
        {
            // bad chunk body
            return (BAD_REQUEST);
        }
        _bodySize = 0;
        _body += line;
        return (CONTINUE);
    }

    StatusCode Request::parseBody(const std::string &line)
    {
        // std::cout << "body:" << std::endl;
        // std::cout << line << std::endl;
        if (_headers.find(TRANSFER_ENCODING) != _headers.end())
        {
            return (parseChunked(line));
        }
        else if (_headers.find(CONTENT_LENGTH) != _headers.end())
        {
            setFlag(PARSED_BODY);
            parseChunked(line);
            // std::cout << "body:" << _body << std::endl;
            // parse
            return PROCESSING;
        }
        // return ;
        return PROCESSING;
    }

    // for chunked
    bool Request::getBodySizeFlag()
    {
        return (_flag_getline_bodySize);
    }

    void Request::setBodySizeFlag(bool isSize)
    {
        _flag_getline_bodySize = isSize;
    }

    unsigned long Request::getBodySize()
    {
        return (_bodySize);
    }
    void Request::setBodySize(unsigned long size)
    {
        _bodySize = size;
    }

    void Request::setStatus(const HTTP::StatusCode &status)
    {
        _status = status;
    }

} // namespace HTTP
