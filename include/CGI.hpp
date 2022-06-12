#pragma once

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <list>
#include <sstream>

#include "Utils.hpp"
#include "Logger.hpp"
#include "ResponseHeader.hpp"

namespace HTTP {

class Request;
class Response;

class CGI {

private:
    std::string _execpath;
    std::string _filepath;
    const char *_args[3];
    bool        _isCompiled;

    Request    *_req;
    Response   *_res;

    size_t            _bodyPos;
    std::stringstream _ss;

    std::list<ResponseHeader> _headers;
    std::list<ResponseHeader> _extraHeaders;

public:
    typedef std::list<ResponseHeader>::iterator iter;
    typedef std::list<ResponseHeader>::iterator const_iter;

    CGI(void);
    ~CGI(void);

    CGI(const CGI &);
    CGI &operator=(const CGI &);

    int  exec(void);
    void setCompiled(bool);
    bool isCompiled(void);

    void linkRequest(Request *req);
    void linkResponse(Response *res);
    void setEnv(void);
    void setFullEnv(void);
    void setExecPath(const std::string);
    bool setScriptPath(const std::string);
    void resetStream(void);
    void clear(void);

    void parseHeaders(void);
    bool isValidContentLength(void);

    const std::list<ResponseHeader> &getHeaders(void) const;
    const std::list<ResponseHeader> &getExtraHeaders(void) const;
    const std::string getExecPath(void) const;
    const std::string getBody(void) const;
    size_t getBodyLength(void) ;
    std::stringstream &getResult(void);

    static const bool extraHeaderEnabled;
    static const std::string compiledExt;
    static const std::string extraHeaderPrefix;

    static char ** const env;
};

}
