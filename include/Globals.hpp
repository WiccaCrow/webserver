#pragma once

#include <map>
#include <string>

// #ifndef SOMAXCONN
//     # define SOMAXCONN 128
// #endif

// #ifndef BLIND_PROXY
//     # define BLIND_PROXY true
// #endif

// #ifndef MAX_REQUESTS
//     # define MAX_REQUESTS 100
// #endif

// #ifndef MAX_CLIENT_TIMEOUT
//     # define MAX_CLIENT_TIMEOUT 25
// #endif

// #ifndef MAX_GATEWAY_TIMEOUT
//     # define MAX_GATEWAY_TIMEOUT 100
// #endif

// #ifndef SESSION_LIFETIME
//     # define SESSION_LIFETIME 1000
// #endif

// #ifndef MAX_URI_LENGTH
//     # define MAX_URI_LENGTH 1024
// #endif

// #ifndef MAX_HEADER_FIELD_LENGTH
//     # define MAX_HEADER_FIELD_LENGTH 2048
// #endif

// #ifndef CHUNK_SIZE
//     # define CHUNK_SIZE 40960
// #endif

#ifndef SERVER_PROTOCOL
    # define SERVER_PROTOCOL "HTTP/1.1"
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

// #ifndef WORKERS
//     # define WORKERS 3
// #endif

// #ifndef WORKER_TIMEOUT
//     # define WORKER_TIMEOUT 10000
// #endif

#ifndef FILE_SIZE
    # define KB 1000
    # define MB 1000 * KB
    # define GB 1000 * MB
    # define TB 1000 * GB
    # define PB 1000 * TB
    # define EB 1000 * PB

    # define KiB 1024
    # define MiB 1024 * KiB
    # define GiB 1024 * MiB
    # define TiB 1024 * GiB
    # define PiB 1024 * TiB
    # define EiB 1024 * PiB
#endif

// #ifndef MAX_RANGE
//     # define MAX_RANGE 2 * MB
// #endif

// #ifndef REGLR_DWNLD_MAX_SIZE
//     # define REGLR_DWNLD_MAX_SIZE 4 * MB
// #endif

class Server;

extern Server *g_server;

extern std::map<std::string, std::string> g_etags;
