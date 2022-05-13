#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <cstdlib>

#include <iostream>
#include <list>
#include <vector>
#include <map>

#include "Logger.hpp"
#include "Location.hpp"

class ServerBlock {
    private:
    // Variables
    std::string         _addr;
    uint16_t            _port;
    int32_t             _servfd;
    // Location            _location_base;
    // std::map<std::string, Location> _locations;
    std::list<std::pair<std::string, std::string> > _cgiPaths;
    std::string _root;

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
