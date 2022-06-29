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

class CGI {

private:
    std::string _execpath;
    std::string _filepath;
    bool        _compiled;
    char      **_env;

    Request    *_req;
    Response   *_res;

public:
    CGI(void);
    ~CGI(void);

    CGI(const CGI &);
    CGI &operator=(const CGI &);

    int  exec(void);
    void compiled(bool);
    bool compiled(void);

    void link(Request *req, Response *res);
    bool setScriptPath(const std::string);

    bool initEnv(void);
    bool setEnv(void);
    bool setFullEnv(void);

    void setExecPath(const std::string);
    const std::string getExecPath(void) const;

    static const std::string compiledExt;
};

}
