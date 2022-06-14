#include "Server.hpp"

const size_t reservedClients = 64;

Server::Server() 
    : _socketsCount(0) {
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
        _serverBlocks = obj._serverBlocks;
        _pollResult = obj._pollResult;
        _pollfds    = obj._pollfds;
        _clients    = obj._clients;
        _socketsCount = obj._socketsCount;
    }
    return (*this);
}

// Private functions

int
Server::addListenSocket(const std::string &addr, size_t port) {
    
    int i = 1;
    struct sockaddr_in data;
    data.sin_family = AF_INET;
    data.sin_port   = htons(port);
    data.sin_addr.s_addr = inet_addr(addr.c_str()); 

    int fd;
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        Log.syserr("Server::socket ->" + addr + ":" + to_string(port));
        exit(1);
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
        Log.syserr("Server::setsockopt ->" + addr + ":" + to_string(port));
        exit(1);
    }
    if (bind(fd, (struct sockaddr *)&data, sizeof(data)) < 0) {
        Log.syserr("Server::bind ->" + addr + ":" + to_string(port));
        exit(1);
    }
    if (listen(fd, SOMAXCONN) < 0) {
        Log.syserr("Server::listen ->" + addr + ":" + to_string(port));
        exit(1);
    }
    
    return fd;
}


void
Server::fillServBlocksFds(void) {
    typedef std::map<size_t, std::set<std::string> >::iterator iter_m;
    typedef std::list<HTTP::ServerBlock>::iterator iter_l;
    typedef std::set<std::string>::iterator iter_s;

    std::map<size_t, std::set<std::string> >uniqueAddr;
    for (iter lst = _serverBlocks.begin(); lst != _serverBlocks.end(); ++lst) {
        std::list<HTTP::ServerBlock> &l = lst->second;
        for (iter_l sb = l.begin() ; sb != l.end(); sb++) {
            uniqueAddr[lst->first].insert(sb->getAddrRef());
        }
    }

    for (iter_m ua = uniqueAddr.begin(); ua != uniqueAddr.end(); ++ua) {
        std::set<std::string> &addrs = ua->second;

        const size_t port = ua->first;
        for (iter_s addr = addrs.begin(); addr != addrs.end(); addr++) {
            _pollfds.push_back((struct pollfd) {
                addListenSocket(*addr, port), POLLIN, 0
            });
            _socketsCount++;
            Log.info("Server::Socket added -> " + *addr + ":" + to_string(port));
        }
    }
}

// Public functions

// Set attributes

// Get and show attributes

// Other methods

void
Server::addServerBlock(HTTP::ServerBlock &servBlock) {
    _serverBlocks[servBlock.getPort()].push_back(servBlock);
}

std::list<HTTP::ServerBlock> &
Server::getServerBlocks(size_t port) {
    return _serverBlocks[port];
}

int
Server::pollInHandler(size_t id) {
    _pollfds[id].revents = 0;
    if (id < _socketsCount) {
        connectClient(id);
        return 1;
    }
    
    if (!_clients[id].getResponse().isFormed()) {
        _clients[id].receive();
    }

    if (_clients[id].getFd() == -1) {
        disconnectClient(id);
    }
    return 0;
}

void
Server::pollHupHandler(size_t id) {
    Log.syserr("POLLHUP occured on the " + to_string(_pollfds[id].fd) + "socket");
    if (id >= _socketsCount) {
        disconnectClient(id);
    }
}

void
Server::pollOutHandler(size_t id) {
    if (_clients[id].getRequest().isFormed()) {
        _clients[id].process();           
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
    Log.syserr("POLLERR occured on the " + to_string(_pollfds[id].fd) + "socket");
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
    Log.syserr(std::string("Poll:: ") + strerror(errno));
    switch (errno) {
        case EINVAL: {
            struct rlimit rlim;
            getrlimit(RLIMIT_NOFILE, &rlim);
            Log.syserr("ndfs: " + to_string(_pollfds.size()));
            Log.syserr("limit(soft): " + to_string(rlim.rlim_cur));
            Log.syserr("limit(hard): " + to_string(rlim.rlim_max));
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
            Log.syserr("Accept::" + std::string(strerror(errno)));
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

    struct sockaddr_in servData;
    socklen_t servLen = sizeof(servData);
    if (getsockname(_pollfds[id].fd, (struct sockaddr *)&servData, &servLen) < 0) {
        Log.syserr("Server::getsockname failed for fd = " + to_string(_pollfds[id].fd));
        return ;
    }

    struct sockaddr_in clientData;
    socklen_t clientLen = sizeof(clientData);
    int fd = accept(_pollfds[id].fd, (struct sockaddr *)&clientData, &clientLen);

    if (fd < 0) {
        handleAcceptError();
        return;
    }

    fcntl(fd, F_SETFL, O_NONBLOCK);

    std::vector<struct pollfd>::iterator it;
    it = std::find_if(_pollfds.begin(), _pollfds.end(), fdFree);

    size_t clientId;
    if (it != _pollfds.end()) {
        it->fd     = fd;
        it->events = POLLIN | POLLOUT;
        clientId   = std::distance(_pollfds.begin(), it);
    } else {
        struct pollfd tmp = { fd, POLLIN | POLLOUT, 0 };
        _pollfds.push_back(tmp);
        clientId = _pollfds.size() - 1;
    }

    _clients.insert(std::make_pair(clientId, HTTP::Client()));
    _clients[clientId].initResponseMethodsHeaders();
    _clients[clientId].linkRequest();
    _clients[clientId].setFd(fd);
    _clients[clientId].setPort(ntohs(clientData.sin_port));
    Log.debug(inet_ntoa(servData.sin_addr));
    _clients[clientId].setIpAddr(inet_ntoa(clientData.sin_addr));
    _clients[clientId].setServerPort(ntohs(servData.sin_port));

    Log.debug("Server::connect [" + to_string(fd) + "] -> " + _clients[clientId].getHostname());
}

void
Server::disconnectClient(size_t id) {
    Log.debug("Server::disconnect [" + to_string(_pollfds[id].fd) + "]");
    _clients.erase(id);
    close(_pollfds[id].fd);
    _pollfds[id].fd      = -1;
    _pollfds[id].events  = 0;
    _pollfds[id].revents = 0;
}

HTTP::ServerBlock *
Server::matchServerBlock(size_t port, const std::string &ipaddr, const std::string &host) {
    (void)ipaddr;

    std::list<HTTP::ServerBlock> &blocks = _serverBlocks[port];
    std::list<HTTP::ServerBlock>::iterator block = blocks.begin();
    std::list<HTTP::ServerBlock>::iterator found = blocks.begin();
    std::list<HTTP::ServerBlock>::iterator end = blocks.end();
    for (; block != end; ++block) {
        std::vector<std::string> &names = block->getServerNamesRef();
        if (std::find(names.begin(), names.end(), host) != names.end()) {
            found = block;
        }
    }
    Log.debug("Server::matchServerBlock -> " + found->getBlockName() + " for " + host + ":" + to_string(port));
    return &(*blocks.begin());
}