#pragma once

#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <list>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utility>

#include "Request.hpp"
#include "ResponseHeader.hpp"

namespace HTTP {

// # define SIZE_FOR_CHUNKED 4096

class Client;

class Response {

    std::string _res;
    std::string _resLeftToSend;
    Request    *_req;
    Client     *_client;
    std::string _body;
    std::string _additionalHeaders;
    size_t      _bodyLength;

public:
    typedef void (Response::*Handler)(void);
    std::map<std::string, Handler> methods;

    std::list<ResponseHeader> headers;

    Response(void);
    ~Response(void);

    void initMethodsHeaders(void);
    void clear(void);

    StatusCode handle(Request &req);

    //  for errors
    static const std::map<std::string, std::string> MIMEs;

    int setErrorResponse(HTTP::StatusCode status);

    // methods

    void DELETE(void);
    void HEAD(void);
    void GET(void);
    void OPTIONS(void);
    void POST(void);
    void PUT(void);

    void        unauthorized(void);
    int         contentForGetHead(void);
    int         redirectForDirectory(const std::string &resourcePath);
    bool        isSetIndexFile(std::string &resourcePath);
    int         makeGetHeadResponseForFile(const std::string &resourcePath);
    int         directoryListing(const std::string &resourcePath);
    int         fileToResponse(std::string resourcePath);
    int         listing(const std::string &resourcePath);
    std::string getContentType(std::string resourcePath);
    void        writeFile(const std::string &resourcePath);

    ResponseHeader *getHeader(HeaderCode code);
    std::string makeHeaders(void);
    void        addHeader(HeaderCode code, const std::string &value);
    void        addHeader(HeaderCode code);

    int passToCGI(CGI &cgi);

    // to send response
    size_t             getResLength(void);
    const char        *getResponse(void);
    const std::string &getBody(void) const;
    void               setBody(const std::string &);
    void               setRequest(Request *req);
    Request           *getRequest(void) const;
    StatusCode         getStatus();
    void               setStatus(HTTP::StatusCode status);

    void            setClient(Client *client);
    Client *        getClient(void);

    // std::string        TransferEncodingChunked(std::string buffer, size_t bufSize);
};

} // namespace HTTP
