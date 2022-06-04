#include "Server.hpp"

const size_t reservedClients = 64;

Server::Server() {
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

// Private functions

int
Server::createListenSocket(const std::string addr, const int port) {
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        Log.error("Server::socket -> addr: " + addr + ":" + to_string(port));
        exit(5);
    }
    
    int i = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
        Log.error("Server::setsockopt -> addr: " + addr + ":" + to_string(port));
        exit(6);
    }

    struct sockaddr_in data;
    data.sin_family      = AF_INET;
    data.sin_port        = htons(port);
    data.sin_addr.s_addr = inet_addr(addr.c_str());
    if (bind(fd, (struct sockaddr *)&data, sizeof(data)) < 0) {
        Log.error("Server::bind -> addr: " + addr + ":" + to_string(port));
        exit(7);
    }

    if (listen(fd, SOMAXCONN) < 0) {
        Log.error("Server::listen -> addr: " + addr + ":" + to_string(port));
        exit(8);
    }
    return fd;
}

void
Server::fillServBlocksFds(void) {
    std::map<int, int> sockets;

    for (size_t i = 0; i < _servBlocks.size(); ++i) {
        const int port = _servBlocks[i].getPort();
        std::map<int, int>::iterator it = sockets.find(port);
        if (it == sockets.end()) {
            int fd = createListenSocket("127.0.0.1", port); // what addr should i use here?
            sockets.insert(std::make_pair(port, fd));
            _servBlocks[i].setFd(fd);
        } else {
            _servBlocks[i].setFd(it->second);
        }

        struct pollfd tmp = {
            _servBlocks[i].getFd(),
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
}

int
Server::pollInHandler(size_t id) {
    _pollfds[id].revents = 0;
    if (id < _servBlocks.size()) {
        connectClient(id);
        return 1;
    } else if (_clients[id].isResponseFormed()){
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
    if (id >= _servBlocks.size()) {
        disconnectClient(id);
    }
}

void
Server::pollOutHandler(size_t id) {
    if (_clients[id].isRequestFormed()) { // Not response, but request formed
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
        return ;
    }

    if (fd > -1) {
        fcntl(fd, F_SETFL, O_NONBLOCK);
       
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
        _clients[clientId].setServerPort(_servBlocks[id].getPort());
        _clients[clientId].setIpAddr(inet_ntoa(clientData.sin_addr));

        Log.debug("Server::connectClient -> fd: " + to_string(fd) + ", addr: " + _clients[clientId].getHostname());  
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
Server::matchServerBlock(int port, const std::string &ipaddr, const std::string &host) {
    (void)ipaddr;
    
    int defaultServerIndex = 0;
    for (int i = _servBlocks.size() - 1; i >= 0; --i) {
        if (_servBlocks[i].getPort() == port) {
            defaultServerIndex = i;

            std::vector<std::string> &sNames = _servBlocks[i].getServerNamesRef();
            if (std::find(sNames.begin(), sNames.end(), host) != sNames.end()) {
                Log.debug("ServerBlock found -> " + _servBlocks[i].getBlockName() + " for: " + host + ":" + to_string(port));
                return &_servBlocks[i];
            }
        }
    }
    Log.debug("ServerBlock found [default] -> " + _servBlocks[defaultServerIndex].getBlockName()  + " for: " + host + ":" + to_string(port));
    return &_servBlocks[defaultServerIndex];
}