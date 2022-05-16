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
    int                 _port;
    int32_t             _servfd;
    std::string         _server_name;
    std::map<int, std::string> _errorPagesPaths;

    
    std::string         _root;
    int                 _post_max_body;
    Location            _locationBase;
    std::map<std::string, std::string> _cgiPaths;
    std::map<std::string, Location> _locations;
    std::vector<std::string> _allowedMethods;
    std::vector<std::string> _index;

    // Methods
    void createSock(void);
    void reuseAddr(void);
    void bindAddr(void);
    void listenSock(void);

    public:
    // Constructs and destructs
    ServerBlock();
    ServerBlock(const std::string &ipaddr, const int port);
    ServerBlock(const ServerBlock &obj);
    ~ServerBlock();

    // Operators
    ServerBlock &operator=(const ServerBlock &obj);

    // Set atributs
    void setAddr(const std::string &);

    // Get and show atributs
    int getServFd(void);

    int &getPortRef(void);
    std::string &getServerNameRef(void);
    std::map<std::string, Location> &getLocationsRef(void);
    Location &getLocationBaseRef(void);
    std::map<int, std::string> &getErrPathsRef(void);


    // std::string &getRootRef(void);
    // int &getPostMaxBodyRef(void);
    // std::map<std::string, std::string> &getCGIPathsRef(void);
    // std::vector<std::string> &getAllowedMethodsRef(void);
    // std::vector<std::string> &getIndexRef(void);
    // bool &getAutoindexRef(void);

    // other methods
    void createListenSock();
};
