#include "ServerBlock.hpp"

namespace HTTP {

ServerBlock::ServerBlock() {}

ServerBlock::ServerBlock(const ServerBlock &obj) {
    operator=(obj);
}

ServerBlock::~ServerBlock() { }

ServerBlock &
ServerBlock::operator=(const ServerBlock &obj) {
    if (this != &obj) {
        _addr            = obj._addr;
        _port            = obj._port;
        _fd              = obj._fd;
        _blockname       = obj._blockname;
        _server_names    = obj._server_names;
        _locationBase    = obj._locationBase;
        _errorPagesPaths = obj._errorPagesPaths;
        _locations       = obj._locations;
    }
    return (*this);
}

int
ServerBlock::getFd(void) {
    return (_fd);
}

void
ServerBlock::setFd(int fd) {
    _fd = fd;
}

void
ServerBlock::setAddr(const std::string &ipaddr) {
    this->_addr = ipaddr;
}

void
ServerBlock::setBlockname(const std::string &blockname) {
    this->_blockname = blockname;
}

const std::string &
ServerBlock::getBlockName(void) const {
    return this->_blockname;
}

std::string &
ServerBlock::getAddrRef(void) {
    return _addr;
}

std::vector<std::string> &
ServerBlock::getServerNamesRef(void) {
    return _server_names;
}

int
ServerBlock::getPort(void) const {
    return _port;
}

int &
ServerBlock::getPortRef(void) {
    return _port;
}

std::map<std::string, Location> &
ServerBlock::getLocationsRef(void) {
    return _locations;
}

const std::map<std::string, Location> &
ServerBlock::getLocationsRef(void) const {
    return _locations;
}

std::map<int, std::string> &
ServerBlock::getErrPathsRef(void) {
    return _errorPagesPaths;
}

Location &
ServerBlock::getLocationBaseRef(void) {
    return _locationBase;
}

Location *
ServerBlock::matchLocation(const std::string &path) {
    size_t matchMaxLen = 0;

    std::map<std::string, Location>::iterator it = _locations.begin();
    std::map<std::string, Location>::iterator end = _locations.end();
    std::map<std::string, Location>::iterator match = _locations.end();
    for (; it != end; it++) {
        size_t len = it->first.length();
        if (path.find(it->first) == 0 && (path.length() == it->first.length() || path[it->first.length()] == '/')) {
            if (len > matchMaxLen) {
                match = it;
                matchMaxLen = len;
            }
        }
    }
    if (match == end) {
        Log.debug("ServerBlock::matchLocation: / for " + path);
        return &_locationBase;
    }
    Log.debug("ServerBlock::matchLocation: " + match->first + " for " + path);
    return &(match->second);
}

}
