#include "Server.hpp"

const size_t reservedClients = 64;

Server::Server() {
    _nbServBlocks = 0;
    _pollfds.reserve(reservedClients);
}

Server::~Server() {
    for (size_t i = 0; i < _pollfds.size(); i++) {
        if (_pollfds[i].fd != -1) {
            close(_pollfds[i].fd);
            _pollfds[i].fd = -1;
        }
    }
}

// Could be used for re-reading config:
// Try to read new configuration into another server,
// and if server is not NULL, then do old_server = new_server,
// else keep old configuration and log message.
Server::Server(const Server &obj) {
    operator=(obj);
}

Server &
Server::operator=(const Server &obj) {
    if (this != &obj) {
        _servBlocks = obj._servBlocks;
        _pollResult = obj._pollResult;
        _pollfds    = obj._pollfds;
        _clients = obj._clients;
    }
    return (*this);
}

std::vector<HTTP::ServerBlock>&
Server::getServerBlocksRef(void) {
    return this->_servBlocks;
}

// Private functions

void
Server::fillServBlocksFds(void) {
    for (size_t i = 0; i < _nbServBlocks; ++i) {
        struct pollfd tmp = {
            _servBlocks[i].getServFd(),
            POLLIN,
            0
        };
        _pollfds.push_back(tmp);
    }
}

// Public functions

// Set attributes

// Get and show attributes

// Other methods

void
Server::addServerBlock(HTTP::ServerBlock &servBlock) {
    _servBlocks.push_back(servBlock);
    _servBlocks.back().createListenSock(); // Should not
    _nbServBlocks++;
}

int
Server::pollInHandler(size_t id) {
    _pollfds[id].revents = 0;
    if (id < _nbServBlocks) {
        connectClient(id);
        return 1;
    } else {
        _clients[id].receive();

        if (_clients[id].getFd() == -1) {
            disconnectClient(id);
        }
        return 0;
    }
}

void
Server::pollHupHandler(size_t id) {
    Log.error("POLLHUP occured on the " + to_string(_pollfds[id].fd) + "socket");
    if (id >= _nbServBlocks) {
        disconnectClient(id);
    }
}

void
Server::pollOutHandler(size_t id) {
    if (_clients[id].responseFormed()) { // Not response, but request formed
        _clients[id].process(); // Response is formed only after process
        _clients[id].reply();
        _clients[id].checkIfFailed();
        _clients[id].clearData();

        if (_clients[id].getFd() == -1) {
            disconnectClient(id);
        }
    }
}

void
Server::pollErrHandler(size_t id) {
    Log.error("POLLERR occured on the " + to_string(_pollfds[id].fd) + "socket");
    exit(1);
}

void
Server::start(void) {
    fillServBlocksFds();
    while (1) {
        _pollResult = 0;

        pollServ();
        const size_t size = _pollfds.size();
        for (size_t id = 0; id < size; id++) {
            if (_pollfds[id].revents & POLLIN) {
                if (pollInHandler(id)) {
                    break;
                }
            } else if (_pollfds[id].revents & POLLHUP) {
                pollHupHandler(id);
            } else if (_pollfds[id].revents & POLLOUT) {
                pollOutHandler(id);
            } else if (_pollfds[id].revents & POLLERR) {
                pollErrHandler(id);
            }
        }
    }
}

void
Server::handlePollError() {
    Log.error(std::string("Poll:: ") + strerror(errno));
    switch (errno) {
        case EINVAL: {
            struct rlimit rlim;
            getrlimit(RLIMIT_NOFILE, &rlim);
            Log.error("ndfs: " + to_string(_pollfds.size()));
            Log.error("limit(soft): " + to_string(rlim.rlim_cur));
            Log.error("limit(hard): " + to_string(rlim.rlim_max));
            break;
        }
        default:
            break;
    }

    // close all fds
    exit(1);
}

void
Server::pollServ(void) {
    while (_pollResult == 0) {
        _pollResult = poll(_pollfds.data(), _pollfds.size(), 1000000);
    }
    if (_pollResult < 0) {
        handlePollError();
    }
}

void
Server::handleAcceptError() {
    switch (errno) {
        case EWOULDBLOCK: {
            // OK, as we use non-blocking sockets
            break;
        }
        default: {
            Log.error("Accept::" + std::string(strerror(errno)));
            break;
        }
    }
}

static int
fdFree(struct pollfd pfd) {
    return (pfd.fd == -1);
}

void
Server::connectClient(size_t id) {
    struct sockaddr_in clientData;

    socklen_t len = sizeof(clientData);

    int fd = accept(_pollfds[id].fd, (struct sockaddr *)&clientData, &len);

    if (fd < 0) {
        handleAcceptError();
        return;
    }

    if (fd > -1) {
        fcntl(fd, F_SETFL, O_NONBLOCK);

        Log.debug("Server::connectClient -> fd: " + to_string(fd));
       
        std::vector<struct pollfd>::iterator it;
        it = std::find_if(_pollfds.begin(), _pollfds.end(), fdFree);

        size_t clientId;

        if (it != _pollfds.end()) {
            it->fd     = fd;
            it->events = POLLIN | POLLOUT;
            clientId = it - _pollfds.begin();
        } else {
            struct pollfd tmp = { fd, POLLIN | POLLOUT, 0 };
            _pollfds.push_back(tmp);
            clientId = _pollfds.size() - 1;
        }

        _clients.insert(std::make_pair(clientId, HTTP::Client()));


        _clients[clientId].linkToRequest();
        _clients[clientId].setFd(fd);
        _clients[clientId].setPort(ntohs(clientData.sin_port));
        _clients[clientId].setIpAddr(inet_ntoa(clientData.sin_addr));

        Log.debug("Server::connectClient -> addr: " + _clients[clientId].getHostname());  

        _clients[clientId].setServerBlock(&_servBlocks[id]); // matchServerBlock will be added 
    }
}

void
Server::disconnectClient(size_t id) {
    Log.debug("Server::disconnectClient -> fd: " + to_string(_pollfds[id].fd));
    _clients.erase(id);
    close(_pollfds[id].fd);
    _pollfds[id].fd = -1;
    _pollfds[id].events = 0;
    _pollfds[id].revents = 0;
}

HTTP::ServerBlock *
matchServerBlock(int port, const std::string &ipaddr, const std::string &host) {
    (void)port;
    (void)ipaddr;
    (void)host;
    return NULL;
}