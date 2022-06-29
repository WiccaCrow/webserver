#pragma once

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <list>
#include <stdlib.h>
#include <unistd.h>
#include <utility>

#include "Request.hpp"
#include "ResponseHeader.hpp"
#include "ErrorResponses.hpp"
#include "Utils.hpp"

namespace HTTP {

// # define CHUNK_SIZE 40960

class AClient;

class AResponse {

public:
    typedef std::list<ResponseHeader> Headers;
    typedef Headers::iterator iterator;
    typedef Headers::const_iterator const_iterator;

private:
    Location    *_location;
    AClient     *_client;
    std::string _body;
    std::string _head;
    size_t      _bodyLength;
    bool        _formed;
    StatusCode  _status;
    Headers     _headers;
    Headers     _mandatory;

public:
    AResponse(void);
    AResponse(Request *req);
    ~AResponse(void);

    AResponse(const AResponse &other);
    AResponse &operator=(const AResponse &other);

    void handle(void);

    ResponseHeader *getHeader(uint32_t hash);
    AClient        *getClient(void);

    void  addHeader(uint32_t hash, const std::string &value);
    void  addHeader(uint32_t hash);

    void  assembleErrorBody(void); // ?
    void  assembleHead(void);
    virtual void  perform(void) = 0;

    void  makeChunk(void);

    void  shouldBeClosedIf(void); // ?

    const std::string &getStatusLine(void) const;
    const std::string &getHead(void) const;
    const std::string &getBody(void) const;
    size_t             getBodyLength(void) const;
    StatusCode         getStatus(void);
    
    void  setHead(const std::string &);
    void  setBody(const std::string &);
    void  setBodyLength(size_t);
    void  setStatus(HTTP::StatusCode status);

    bool formed(void) const;
    void formed(bool);
};

} // namespace HTTP
