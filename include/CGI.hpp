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
#include "Request.hpp"

// namespace HTTP {

// Interpreted scripts
// cgi.execpath          cgi.args                     cgi.env 
// /usr/bin/python       scr.py var1 var2 NULL        var1, var2, NULL

// Compiled programs
// cgi.execpath          cgi.args                     cgi.env 
// scr.cgi               scr.cgi var1 var2 NULL       var1, var2, NULL

// .cgi script should be executable

// class CGI
// {
//     private:
//         std::string _ext;
//         std::string _filepath;
//         std::string _execpath;
//         // std::vector<std::string> _args;
//     public:
//         CGI(void);
//         ~CGI(void);
//         std::string exec(void);
// };
    
//     CGI::CGI(){}
//     CGI::~CGI(){}
//     std::string CGI::exec() {return "";}
    
// }

std::string CGI(HTTP::Request &req, std::map<std::string, std::string>::const_iterator it);
