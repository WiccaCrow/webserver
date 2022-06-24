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

#ifndef FILE_SIZE
    #define KB 1024
    #define MB 1024 * KB
    #define GB 1024 * MB
    #define TB 1024 * GB
    #define PB 1024 * TB
    #define EB 1024 * PB
#endif

#ifndef MAX_RANGE
    # define MAX_RANGE 2 * MB
#endif

#ifndef REGLR_DWNLD_MAX_SIZE
    # define REGLR_DWNLD_MAX_SIZE 4 * MB
#endif

class Server;

extern bool finished;

extern Server *g_server;

extern std::map<std::string, std::string> g_etags;
