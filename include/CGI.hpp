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
#include <vector>

#include "Utils.hpp"
// #include "Request.hpp"
#include "Logger.hpp"
// #include "Globals.hpp"

namespace HTTP {

class Request;

class CGI {

private:
    std::string _execpath;
    std::string _filepath;
    const char *_args[3];

    bool _isCompiled;

    std::string _res;

public:
    CGI(void);
    ~CGI(void);

    int  exec(void);
    void setCompiled(bool);
    bool isCompiled(void);

    void setEnv(Request &req);
    void setFullEnv(Request &req);
    void setExecPath(const std::string);
    bool setScriptPath(const std::string);
    void reset(void);

    const std::string getExecPath(void) const;
    const std::string getResult(void) const;

    static const std::string compiledExt;

    static char **env;
};

}
