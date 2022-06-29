#pragma once

#include "ResponseHeader.hpp"
#include "Status.hpp"
#include <list>
#include "ClientCGI.hpp"

namespace HTTP {

class ResponseCGI {

private:
    ClientCGI  *_client;
    std::string _body;
    std::string _head;
    size_t      _bodyLength;
    bool        _isFormed;
    StatusCode  _status;

    typedef std::list<ResponseHeader>::iterator iter;
    typedef std::list<ResponseHeader>::iterator const_iter;

    std::list<ResponseHeader> _headers;
    std::list<ResponseHeader> _extraHeaders;

public:
    ResponseCGI();
    ~ResponseCGI();

    void parse(std::string &line);

    const std::string getBody(void) const;
    size_t getBodyLength(void);

    static const bool extraHeadersEnabled;
    static const std::string extraHeaderPrefix;
};

}
