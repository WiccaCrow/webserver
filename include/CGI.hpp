#pragma once

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "Utils.hpp"
// #include "Request.hpp"
#include "Logger.hpp"
// #include "Globals.hpp"


namespace HTTP
{
    class Request;

    class CGI
    {
    private:
        const char *_execpath;
        const char *_args[2];
        const char *_env[20];

        bool _isCompiled;

        std::string _res;

    public:
        CGI(void);
        ~CGI(void);

        int exec(void);
        void setCompiled(bool);
        bool isCompiled(void);

        void setEnv(Request &req);
        void setExecPath(const std::string &);
        const std::string getExecPath(void) const;

        bool setScriptPath(const std::string &);

        static const std::string compiledExt;
    };

}
