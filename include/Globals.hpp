#pragma once

#include <map>
#include <string>

#ifndef SOMAXCONN
    # define SOMAXCONN 128
#endif

#ifndef BLIND_PROXY
    # define BLIND_PROXY true
#endif

#ifndef MAX_REQUESTS
    # define MAX_REQUESTS 100
#endif

#ifndef MAX_CLIENT_TIMEOUT
    # define MAX_CLIENT_TIMEOUT 5
#endif

#ifndef MAX_GATEWAY_TIMEOUT
    # define MAX_GATEWAY_TIMEOUT 100
#endif

#ifndef SESSION_LIFETIME
    # define SESSION_LIFETIME 1000
#endif

#ifndef CHUNK_SIZE
    # define CHUNK_SIZE 40960
#endif

#ifndef SERVER_PROTOCOL
    # define SERVER_PROTOCOL "HTTP/1.1"
#endif

#ifndef SP
    # define SP " "
#endif

#ifndef HTAB
    # define HTAB "\t"
#endif

#ifndef CR
    # define CR "\r"
#endif

#ifndef LF
    # define LF "\n"
#endif

#ifndef CRLF
    # define CRLF CR LF
#endif

#ifndef SERVER_SOFTWARE 
    # define SERVER_SOFTWARE "webserv/1.0.0"
#endif

#ifndef GATEWAY_INTERFACE
    # define GATEWAY_INTERFACE "CGI/1.1"
#endif

#ifndef DEFAULT_CONF_PATH
    # define DEFAULT_CONF_PATH "./default/conf.json"
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

extern Server *g_server;

extern std::map<std::string, std::string> g_etags;
