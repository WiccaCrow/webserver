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
            int fd = addListenSocket(*addr, port);
            _pollfds.push_back((struct pollfd) {fd, POLLIN, 0});
            _socketsCount++;
            Log.info("Server::Listen [" + to_string(fd) + "] -> " + *addr + ":" + to_string(port));
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

void
Server::pollInHandler(size_t id) {
    
    if (_clients[id].reqPoolReady()) {
        _clients[id].addRequest();
    }

    if (!_clients[id].getTopRequest().isFormed()) {
        _clients[id].receive();
    }

    if (_clients[id].getTopRequest().isFormed()) {
        _clients[id].addResponse();
    }

    if (_clients[id].getFd() == -1) {
        disconnectClient(id);
    }
}

void
Server::pollHupHandler(size_t id) {
    Log.syserr("Server::POLLHUP occured on the " + to_string(_pollfds[id].fd) + "socket");
    if (id >= _socketsCount) {
        disconnectClient(id);
    }
}

void
Server::pollOutHandler(size_t id) {
    if (_clients[id].couldProcess()) {
        _clients[id].process();
    }

    if (_clients[id].couldReply()) {
        _clients[id].reply();
        _clients[id].checkIfFailed();
        _clients[id].removeTopRequest();
        _clients[id].removeTopResponse();
    }

    if (_clients[id].getFd() == -1) {
        disconnectClient(id);
    }
}

void
Server::pollErrHandler(size_t id) {
    Log.syserr("Server::POLLERR occured on the " + to_string(_pollfds[id].fd) + "socket");
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
                _pollfds[id].revents = 0;
                id >= _socketsCount ? 
                pollInHandler(id) : connectClient(id);
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
    Log.syserr("Server::poll");
    switch (errno) {
        case EINVAL: {
            struct rlimit rlim;
            getrlimit(RLIMIT_NOFILE, &rlim);
            Log.debug("ndfs: " + to_string(_pollfds.size()));
            Log.debug("limit(soft): " + to_string(rlim.rlim_cur));
            Log.debug("limit(hard): " + to_string(rlim.rlim_max));
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

static int
fdFree(struct pollfd pfd) {
    return (pfd.fd == -1);
}

void
Server::connectClient(size_t servid) {

    struct sockaddr_in servData;
    socklen_t servLen = sizeof(servData);
    if (getsockname(_pollfds[servid].fd, (struct sockaddr *)&servData, &servLen) < 0) {
        Log.syserr("Server::getsockname failed for fd = " + to_string(_pollfds[servid].fd));
        return ;
    }

    struct sockaddr_in clientData;
    socklen_t clientLen = sizeof(clientData);
    int fd = accept(_pollfds[servid].fd, (struct sockaddr *)&clientData, &clientLen);

    if (fd < 0) {
        Log.syserr("Server::accept");
        return;
    }

    fcntl(fd, F_SETFL, O_NONBLOCK);

    size_t id;
    std::vector<struct pollfd>::iterator it;
    it = std::find_if(_pollfds.begin(), _pollfds.end(), fdFree);
    if (it != _pollfds.end()) {
        it->fd     = fd;
        it->events = POLLIN | POLLOUT;
        id   = std::distance(_pollfds.begin(), it);
    } else {
        _pollfds.push_back((struct pollfd) { fd, POLLIN | POLLOUT, 0 });
        id = _pollfds.size() - 1;
    }

    _clients.insert(std::make_pair(id, HTTP::Client()));
    _clients[id].setFd(fd);
    _clients[id].setPort(ntohs(clientData.sin_port));
    _clients[id].setIpAddr(inet_ntoa(clientData.sin_addr));
    _clients[id].setServerPort(ntohs(servData.sin_port));
    _clients[id].setServerIpAddr(inet_ntoa(servData.sin_addr));

    Log.debug("Server::connect [" + to_string(fd) + "] -> " + _clients[id].getHostname());
}

void
Server::disconnectClient(size_t id) {
    const int fd = _pollfds[id].fd;

    close(fd);
    _clients.erase(id);
    _pollfds[id].fd      = -1;
    _pollfds[id].events  = 0;
    _pollfds[id].revents = 0;
    
    Log.debug("Server::disconnect [" + to_string(fd) + "]");
}
