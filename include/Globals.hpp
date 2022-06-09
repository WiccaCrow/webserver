#pragma once

#include <map>
#include <string>

#ifndef SERVER_SOFTWARE
    # define SERVER_SOFTWARE "webserv/1.0.0"
#endif

#ifndef GATEWAY_INTERFACE
    # define GATEWAY_INTERFACE "CGI/1.1"
#endif

class Server;

extern Server *g_server;

extern std::map<std::string, std::string> g_etags;
