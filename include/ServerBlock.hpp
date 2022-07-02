#pragma once

#include <map>
#include <cstdlib>
#include <iostream>

#include "Logger.hpp"
#include "Location.hpp"

namespace HTTP {

class ServerBlock {

public:
    typedef std::map<std::string, Location>  LocationsMap;
    typedef std::vector<std::string>         ServerNamesVec;

private:
    std::string     _blockname;
    std::string     _addr;
    int             _port;
    LocationsMap    _locations;
    Location        _locationBase;
    ServerNamesVec  _server_names;

public:
    ServerBlock();
    ServerBlock(const ServerBlock &);
    ~ServerBlock();

    ServerBlock &operator=(const ServerBlock &);

    void setAddr(const std::string &);
    void setBlockname(const std::string &);

    std::string         &getAddrRef(void);
    int                 &getPortRef(void);
    int                 getPort(void) const;
    Location            &getLocationBaseRef(void);
    ServerNamesVec      &getServerNamesRef(void);
    LocationsMap        &getLocationsRef(void);
    const LocationsMap  &getLocationsRef(void) const;
    const std::string   &getBlockName(void) const;
    
    bool hasName(const std::string &) const;
    bool hasAddr(const std::string &) const;

    Location *matchLocation(const std::string &path);
};

}
