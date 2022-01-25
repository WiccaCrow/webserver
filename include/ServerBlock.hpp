#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <cstdlib>

//#include <cstdint>
#include <iostream>

#include "Logger.hpp"

// enum AllowedMethods
//{
//     GET = 1,
//     PUT = 2,
//     POST = 4,
//     HEAD = 8,
//     TRACE = 16,
//     PATCH = 32,
//     DELETE = 64,
//     CONNECT = 128,
//     OPTIONS = 256
// };

// class Location
//{
//     int16_t allowedMethods;
//     std::string path;
//     bool autoindex;
//     std::string defaultResponseFile;
//     CGI
// };

class ServerBlock {
    private:
    // Variables
    std::string _addr;
    uint16_t    _port;
    // std::string                 _uri; //from config
    int32_t _servfd;
    // std::vector<Location> locations;

    // Methods
    void createSock(void);
    void reuseAddr(void);
    void bindAddr(void);
    void listenSock(void);

    public:
    // Constructs and destructs
    ServerBlock();
    ServerBlock(const std::string &ipaddr, const uint16_t port);
    ServerBlock(const ServerBlock &obj);
    ~ServerBlock();

    // Operators
    ServerBlock &operator=(const ServerBlock &obj);
    // Set atributs

    // Get and show atributs
    int getServFd(void);

    // other methods
    void createListenSock();
};
