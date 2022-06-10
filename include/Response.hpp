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

#include "ResponseHeader.hpp"
#include "Request.hpp"
#include "Status.hpp"
#include "Utils.hpp"
#include "CRC.hpp"

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

public:
    typedef void (Response::*Handler)(void);
    std::map<std::string, Handler> methods;

    // std::map<uint32_t, ResponseHeader> headers;
    std::list<ResponseHeader> headers;

    Response(void);
    ~Response(void);

    void initMethodsHeaders(void);
    void clear(void);

    StatusCode handle(Request &req);

    //  for errors
    static const std::map<std::string, std::string>       MIMEs;
    // static const std::map<HTTP::StatusCode, std::string> errorResponses;

    int setErrorResponse(HTTP::StatusCode status);

    // methods

    void DELETE(void);
    void HEAD(void);
    void GET(void);
    void OPTIONS(void);
    void POST(void);
    void PUT(void);

    // void        generateHeaders(Request &req);
    int         contentForGetHead(void);
    int         fileToResponse(std::string resourcePath);
    int         listing(const std::string &resourcePath);
    std::string getContentType(std::string resourcePath);
    void        writeFile(const std::string &resourcePath);

    std::string makeHeaders(void);
    void        addHeader(HeaderCode code, std::string value);
    void        addHeader(HeaderCode code);

    int passToCGI(CGI &cgi);

    // to send response
    size_t             getResLength(void);
    const char        *getResponse(void);
    const std::string &getBody(void) const;
    void               setRequest(Request *req);
    Request           *getRequest(void) const;
    StatusCode         getStatus();
    void               setStatus(HTTP::StatusCode status);

    void            setClient(Client *client);
    Client *        getClient(void);

    // std::string        TransferEncodingChunked(std::string buffer, size_t bufSize);
};

} // namespace HTTP
