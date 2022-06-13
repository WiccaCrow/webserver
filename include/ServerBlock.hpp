#pragma once

#include <map>
#include <cstdlib>
#include <iostream>

#include "Logger.hpp"
#include "Location.hpp"

namespace HTTP {

class ServerBlock {

private:
    std::string              _blockname;
    std::string              _addr;
    int                      _port;
    // int                      _fd;
    std::vector<std::string> _server_names;

    Location                        _locationBase;
    std::map<int, std::string>      _errorPagesPaths;
    std::map<std::string, Location> _locations;

    // Methods

public:
    ServerBlock();
    ServerBlock(const ServerBlock &obj);
    ~ServerBlock();

    // Operators
    ServerBlock &operator=(const ServerBlock &obj);

    // Set atributs
    // void setFd(int fd);
    void setAddr(const std::string &);
    void setBlockname(const std::string &);

    // Get and show atributs
    int          getPort(void) const;
    // int          getFd(void);


    std::string                           &getAddrRef(void);
    int                                   &getPortRef(void);
    Location                              &getLocationBaseRef(void);
    std::vector<std::string>              &getServerNamesRef(void);
    std::map<int, std::string>            &getErrPathsRef(void);
    std::map<std::string, Location>       &getLocationsRef(void);
    const std::map<std::string, Location> &getLocationsRef(void) const;
    const std::string                     &getBlockName(void) const;

    // other methods
    Location *matchLocation(const std::string &path);
};

}
