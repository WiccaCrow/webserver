#pragma once

#include <map>
#include <string>

#ifndef SERVER_SOFTWARE
    # define SERVER_SOFTWARE "webserv/1.0.0"
#endif

#ifndef GATEWAY_INTERFACE
    # define GATEWAY_INTERFACE "CGI/1.1"
#endif

#ifndef WORKERS
    # define WORKERS 3
#endif

#ifndef WORKER_TIMEOUT
    # define WORKER_TIMEOUT 10000
#endif

class Server;

extern bool finished;

extern Server *g_server;

extern std::map<std::string, std::string> g_etags;
