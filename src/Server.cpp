#include "Server.hpp"

const size_t reservedClients = 64;

// Declared in Globals.hpp
bool finished;

Server::Server() 
    : _socketsCount(0) {
    _pollfds.reserve(reservedClients);

    for (size_t i = 0; i < WORKERS; i++) {
        wPoolCtls[i].id = i;
    }
    
}

Server::~Server() {
    for (size_t i = 0; i < _pollfds.size(); i++) {
        if (_pollfds[i].fd != -1) {
            close(_pollfds[i].fd);
            _pollfds[i].fd = -1;
        }
    }
    // Not the best solution, but fine for current purposes
    // Good choice is to handle SIG_INT
    PoolController::mutexDestroy(); 
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
        Log.syserr() << "Server::socket ->" << addr << ":" << port << std::endl;
        exit(1);
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
        Log.syserr() << "Server::setsockopt ->" << addr << ":" << port << std::endl;
        exit(1);
    }
    if (bind(fd, (struct sockaddr *)&data, sizeof(data)) < 0) {
        Log.syserr() << "Server::bind ->" << addr << ":" << port << std::endl;
        exit(1);
    }
    if (listen(fd, SOMAXCONN) < 0) {
        Log.syserr() << "Server::listen ->" << addr << ":" << port << std::endl;
        exit(1);
    }
    
    return fd;
}

void *worker_cycle(void *ptr) {
    PoolController *poolCtl = reinterpret_cast<PoolController *>(ptr);

    if (poolCtl == NULL) {
        Log.error() << "Invalid poolController" << std::endl;
        return NULL; // or exit ?
    }

    while (!finished) {

        HTTP::Request *req = poolCtl->getRequest();

        if (req == NULL) {
            usleep(WORKER_TIMEOUT);
            continue ;
        } else {
            HTTP::Response *res = new HTTP::Response(req);
            if (res != NULL) {
                Log.debug() << "Worker " << poolCtl->id << ": processing request" << std::endl; 
                res->handle();
                poolCtl->putResponse(res);
            } else {
                Log.syserr() << "Cannot allocate memory for Response" << std::endl;
            }
        }
    }

    return NULL;
}

void
Server::createWorkers(void) {
    PoolController::mutexInit();

    finished = false;
    for (size_t i = 0; i < WORKERS; i++) {
        if (pthread_create(&_threads[i], NULL, worker_cycle, &wPoolCtls[i])) {
            Log.syserr() << "Server::pthread create failed for worker" << i << std::endl;
        } else {
            Log.debug() << "Server:: Worker " << i << " created" << std::endl;
        }
    }
}

void
Server::destroyWorkers(void) {
    
    finished = true;
    for (size_t i = 0; i < WORKERS; i++) {
        if (pthread_join(_threads[i], NULL)) {
            Log.syserr() << "Server::pthread join failed for worker" << i << std::endl;
        } else {
            Log.debug() << "Server:: Worker " << i << " destroyed" << std::endl;
        }
    }
}

void
Server::freePool(void) {
    
    HTTP::Response *res = NULL;

    do {
        res = poolCtl.getResponse();
        if (res != NULL) {
            res->getClient()->addResponse(res);
        }

    } while (res != NULL);

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
            Log.info() << "Server::listen [" << fd << "] -> " << *addr << ":" << port << std::endl;
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
    
    if (_clients[id].requestPoolReady()) {
        _clients[id].addRequest();
    }

    if (!_clients[id].requestReady()) {
        _clients[id].receive();
    }

    if (_clients[id].requestReady()) {
        HTTP::Request *req = NULL;
        
        req = _clients[id].getTopRequest();
        _clients[id].removeTopRequest();
        poolCtl.putRequest(req);
        _clients[id].requestPoolReady(true);
    }

    if (!_clients[id].validSocket()) {
        disconnectClient(id);
    }
}

void
Server::pollHupHandler(size_t id) {
    Log.syserr() << "Server::POLLHUP occured on the " << _pollfds[id].fd << "socket" << std::endl;
    if (id >= _socketsCount) {
        disconnectClient(id);
    }
}

// Do not remove client until all responses performed (returned from workers) !
void
Server::pollOutHandler(size_t id) {

    if (_clients[id].replyReady() && !_clients[id].replyDone()) {
        _clients[id].reply();
    }
    
    if (_clients[id].replyDone()) {
        _clients[id].checkIfFailed();
        _clients[id].removeTopResponse();
        _clients[id].replyDone(false);
    }

    if (!_clients[id].validSocket()) {
        disconnectClient(id);
    }
}

void
Server::pollErrHandler(size_t id) {
    Log.syserr() << "Server::POLLERR occured on the " << _pollfds[id].fd << "socket" << std::endl;
    exit(1);
}

HTTP::Client *
Server::getClient(size_t id) {
    return &_clients[id];
}

void
Server::start(void) {
    fillServBlocksFds();
    createWorkers();
    while (1) {
        freePool();
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
    destroyWorkers();
}

void
Server::handlePollError() {
    Log.syserr() << "Server::poll" << std::endl;
    switch (errno) {
        case EINVAL: {
            struct rlimit rlim;
            getrlimit(RLIMIT_NOFILE, &rlim);
            Log.debug() << "ndfs: " << _pollfds.size() << std::endl;
            Log.debug() << "limit(soft): " << rlim.rlim_cur << std::endl;
            Log.debug() << "limit(hard): " << rlim.rlim_max << std::endl;
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
        Log.syserr() << "Server::getsockname failed for fd = " << _pollfds[servid].fd << std::endl;
        return ;
    }

    struct sockaddr_in clientData;
    socklen_t clientLen = sizeof(clientData);
    int fd = accept(_pollfds[servid].fd, (struct sockaddr *)&clientData, &clientLen);

    if (fd < 0) {
        Log.syserr() << "Server::accept" << std::endl;
        return;
    }

    size_t id = addSockToPollfd(fd);
    addClient(id, fd, &clientData, &servData);
    Log.debug() << "Server::connect [" << fd << "] -> " << _clients[id].getHostname() << std::endl;
}

size_t
Server::proxySetFdAndClient(int fd, 
                              struct sockaddr_in *clientDataIp4) {
    size_t id = addSockToPollfd(fd);
    addClient(id, fd, clientDataIp4 , NULL);
    _clients[id].getProxy()->on(true);
    return id;
}

size_t
Server::addClient(size_t id, int fd,
                  struct sockaddr_in *clientDataIp4,
                  struct sockaddr_in *servData) {
    _clients.insert(std::make_pair(id, HTTP::Client()));
    _clients[id].setId(id);
    _clients[id].setFdIn(fd);
    _clients[id].setPort(ntohs(clientDataIp4->sin_port));
    _clients[id].setIpAddr(inet_ntoa(clientDataIp4->sin_addr));
    if (servData) {
        _clients[id].setServerPort(ntohs(servData->sin_port));
        _clients[id].setServerIpAddr(inet_ntoa(servData->sin_addr));
    }
    return id;
}

size_t
Server::addSockToPollfd(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);

    size_t id;
    std::vector<struct pollfd>::iterator it;
    it = std::find_if(_pollfds.begin(), _pollfds.end(), fdFree);
    if (it != _pollfds.end()) {
        it->fd     = fd;
        it->events = POLLIN | POLLOUT;
        id = std::distance(_pollfds.begin(), it);
    } else {
        _pollfds.push_back((struct pollfd) { fd, POLLIN | POLLOUT, 0 });
        id = _pollfds.size() - 1;
    }

    return id;
}

void
Server::disconnectClient(size_t id) {
    if (_clients[id].isProxy()) {
        size_t idOtherSide = _clients[id].getProxy()->idOtherSide();
        disconnectClientOneSide(idOtherSide);
    }
    disconnectClientOneSide(id);
}

void
Server::disconnectClientOneSide(size_t id) {
    const int fd = _pollfds[id].fd;

    close(fd);
    _clients.erase(id);
    _pollfds[id].fd      = -1;
    _pollfds[id].events  = 0;
    _pollfds[id].revents = 0;
    
    Log.debug() << "Server::disconnect [" << fd << "]" << std::endl;
}
