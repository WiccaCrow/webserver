#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <cstdlib>

#include <iostream>
#include <map>

#include "Logger.hpp"
#include "Location.hpp"

namespace HTTP
{
    class ServerBlock
    {
    private:
        // Variables
        std::string _blockname;
        std::string _addr;
        int _port;
        int32_t _servfd;
        std::string _server_name;

        Location _locationBase;
        std::map<int, std::string> _errorPagesPaths;
        std::map<std::string, Location> _locations;

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
        void setBlockname(const std::string &);

        // Get and show atributs
        int getPort(void) const;
        int getServFd(void);
        std::string &getAddrRef(void);

        int &getPortRef(void);
        Location &getLocationBaseRef(void);
        std::string &getServerNameRef(void);
        std::map<int, std::string> &getErrPathsRef(void);
        std::map<std::string, Location> &getLocationsRef(void);
        const std::map<std::string, Location> &getLocationsRef(void) const;

        // other methods
        void createListenSock();
    };

}
