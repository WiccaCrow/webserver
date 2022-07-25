#pragma once

#include <map>
#include <string>

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

#ifndef TIME_STAMP
    #define SEC 1
    #define MIN 60 * SEC
    #define HOUR 60 * MIN
    #define DAY 24 * HOUR
    #define MONTH 30 * DAY
    #define YEAR 12 * MONTH
#endif

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

class Server;
extern Server *g_server;

