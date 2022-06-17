#pragma once

#include <cstdio>
#include <dirent.h>
#include <iostream>
#include <fstream>
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

# define CHUNK_SIZE 40960

class Client;

class Response {

    std::string _res;
    std::string _resLeftToSend;
    Request    *_req;
    Client     *_client;
    CGI        *_cgi;
    std::string _body;
    std::string _extraHeaders;
    size_t      _bodyLength;
    bool        _isFormed;
    StatusCode  _status;

    std::ifstream _resourceFileStream;

public:
    typedef void (Response::*Handler)(void);
    std::map<std::string, Handler> methods;

    typedef std::list<ResponseHeader>::iterator iter;
    typedef std::list<ResponseHeader>::const_iterator const_iter;
    std::list<ResponseHeader> headers;

    Response(void);
    Response(Request &req);
    ~Response(void);

    Response(const Response &other);
    Response &operator=(const Response &other);

    void initMethodsHeaders(void);
    void clear(void);

    void handle(void);

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
    int         openFileToResponse(std::string resourcePath);
    int         listing(const std::string &resourcePath);
    std::string getContentType(std::string resourcePath);
    void        writeFile(const std::string &resourcePath);

    ResponseHeader *getHeader(uint32_t hash);
    std::string     makeHeaders(void);
    void            addHeader(uint32_t hash, const std::string &value);
    void            addHeader(uint32_t hash);

    std::map<std::string, CGI>::iterator
        isCGI(const std::string &filepath, std::map<std::string, CGI> &cgis);
    
    int passToCGI(CGI &cgi);
    int recognizeHeaders(CGI &cgi);

    void  makeChunk();

    // setters, getters
    // to send response
    size_t             getResponseLength(void);
    const std::string &getResponse(void);
    const std::string &getBody(void) const;
    void               setBody(const std::string &);
    size_t             getBodyLength(void) const;
    void               setBodyLength(size_t);
    Request           *getRequest(void);
    const std::string &getStatusLine(void);
    StatusCode         getStatus();
    void               setStatus(HTTP::StatusCode status);

    Client *        getClient(void);

    bool            isFormed(void) const;
    void            isFormed(bool formed);

    std::string getEtagFile(const std::string &filename);
};

} // namespace HTTP
