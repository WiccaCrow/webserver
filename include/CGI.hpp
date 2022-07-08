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

    // Exec vars
    char      **_env;
    int         _childPID;

public:
    CGI(void);
    ~CGI(void);

    CGI(const CGI &);
    CGI &operator=(const CGI &);

    int  exec(Response *);
    void compiled(bool);
    bool compiled(void);

    int getPID(void) const;
    void setPID(int);

    void setScriptPath(const std::string &);

    bool initEnv(void);
    bool setEnv(Request *);
    bool setFullEnv(Request *);

    void setExecPath(const std::string);
    const std::string getExecPath(void) const;

    static const std::string compiledExt;
};

}
