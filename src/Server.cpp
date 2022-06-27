#include "Server.hpp"
#include "Pool.hpp"

const size_t reservedClients = 64;

typedef Pool<HTTP::Request *>  RequestPool;
typedef Pool<HTTP::Response *> ResponsePool;

// Thread-safe container
RequestPool requests;
ResponsePool responses;

Server::Server() 
    : _socketsCount(0), _working(true) {
    _pollfds.reserve(reservedClients);
    _clients.reserve(reservedClients);

    for (size_t i = 0; i < WORKERS; i++) {
        _workerInfos[i].id = i;
    }
}

Server::~Server() {
    for (size_t i = 0; i < _pollfds.size(); i++) {
        if (_pollfds[i].fd != -1) {
            close(_pollfds[i].fd);
            _pollfds[i].fd = -1;
        }
    }

    for (size_t i = 0; i < _clients.size(); i++) {
        if (_clients[i] != NULL) {
            delete _clients[i];
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
        _pollfds    = obj._pollfds;
        _clients    = obj._clients;
        _socketsCount = obj._socketsCount;
    }
    return (*this);
}

bool
Server::isWorking(void) {
    return _working;
}

// Private functions

int
Server::createListenSocket(const std::string &addr, size_t port) {
    
    int i = 1;
    struct sockaddr_in data;
    data.sin_family = AF_INET;
    data.sin_port   = htons(port);
    data.sin_addr.s_addr = inet_addr(addr.c_str());

    int fd;
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        Log.syserr() << "Server::socket ->" << addr << ":" << port << Log.endl;
        return -1;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
        Log.syserr() << "Server::setsockopt ->" << addr << ":" << port << Log.endl;
        return -1;
    }
    if (bind(fd, (struct sockaddr *)&data, sizeof(data)) < 0) {
        Log.syserr() << "Server::bind ->" << addr << ":" << port << Log.endl;
        return -1;
    }
    if (listen(fd, SOMAXCONN) < 0) {
        Log.syserr() << "Server::listen ->" << addr << ":" << port << Log.endl;
        return -1;
    }
    
    return fd;
}

void *workerCycle(void *ptr) {

    WorkerInfo *w = reinterpret_cast<WorkerInfo *>(ptr);

    Log.debug() << "Worker:: [" << w->id << "] created" << Log.endl;

    while (g_server->isWorking()) {
    
        try {
            HTTP::Request *req = requests.pop_front();
            HTTP::Response *res = new HTTP::Response(req);

            if (res == NULL) {
                Log.syserr() << "Cannot allocate memory for Response" << Log.endl;
                // Returning req back to pool, so worker could try again. 
                requests.push_back(req);
                usleep(WORKER_TIMEOUT);
                continue ;
            }

            const std::string &path = req->getUriRef()._path;

            Log.debug() << "Worker:: [" << w->id << "] -> " << path << " started" << Log.endl; 
            res->handle();
            Log.debug() << "Worker:: [" << w->id << "] -> " << path << " finished" << Log.endl; 
            responses.push_back(res);

        } catch (RequestPool::Empty &e) {            
            usleep(WORKER_TIMEOUT);
            continue ;
        }
    }
    Log.debug() << "Worker:: [" << w->id << "] destroyed" << Log.endl;
    return NULL;
}

void
Server::createWorkers(void) {

    if (!isWorking()) {
        return ;
    }

    for (size_t i = 0; i < WORKERS; i++) {
        if (pthread_create(&_threads[i], NULL, workerCycle, &_workerInfos[i])) {
            Log.syserr() << "Server::pthread_create failed for worker " << _workerInfos[i].id << Log.endl;
        }
    }
}

void
Server::destroyWorkers(void) {

    for (size_t i = 0; i < WORKERS; i++) {
        if (pthread_join(_threads[i], NULL)) {
            Log.syserr() << "Server::pthread_join failed for worker " << _workerInfos[i].id << Log.endl;
        }
    }
}

void
Server::freeResponsePool(void) {
    
    do {
        try {
            HTTP::Response *res = responses.pop_front();
            res->getClient()->addResponse(res);
        } catch (ResponsePool::Empty &e) {            
            return ;
        }
    } while (!responses.empty());

}

int 
Server::addListenSocket(const std::string &addr, size_t port) {
    int fd = createListenSocket(addr, port);

    Log.info() << "Server::listen [" << fd << "] -> " << addr << ":" << port << Log.endl;
    if (fd >= 0) {
        addSocket((struct pollfd) { fd, POLLIN, 0 });
        _socketsCount++;
    }
    return fd;
}

void
Server::createSockets(void) {

    typedef std::set<std::string> uniqueAddr;
    typedef uniqueAddr::iterator iter_ua;

    typedef std::map<size_t, uniqueAddr> uniqueValMap;
    typedef uniqueValMap::iterator iter_uvm;

    if (!isWorking()) {
        return ;
    }

    uniqueValMap uniqueAddrs;
    for (iter_sm it = _serverBlocks.begin(); it != _serverBlocks.end(); ++it) {
        for (iter_sl sb = it->second.begin(); sb != it->second.end(); ++sb) {
            const size_t port = it->first;
            const std::string &addr = sb->getAddrRef();
            uniqueAddrs[port].insert(addr);
        }
    }

    for (iter_uvm it = uniqueAddrs.begin(); it != uniqueAddrs.end(); ++it) {
        for (iter_ua ua = it->second.begin(); ua != it->second.end(); ++ua) {
            const size_t port = it->first;
            const std::string &addr = *ua;
            if (addListenSocket(addr, port) < 0) {
                finish();
                return ;
            }
        }
    }
}

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
    
    HTTP::Client *client = _clients[id];

    if (client == NULL) {
        return ;
    }

    if (!client->getRequest()) {
        client->addRequest();
    }

    if (!client->requestReady()) {
        client->receive();
    }

    if (client->requestReady()) {
        requests.push_back(client->getRequest());
        client->setRequest(NULL);
    }

    if (!client->validSocket()) {
        disconnectClient(id);
    }
}

// Do not remove client until all responses performed (returned from workers) !
// Or check presence of the client in freePool
void
Server::pollOutHandler(size_t id) {
    
    HTTP::Client *client = _clients[id];

    if (client == NULL) {
        return ;
    }

    if (client->replyReady() && !client->replyDone()) {
        client->reply();
    }
    
    if (client->replyDone()) {
        client->removeResponse();
        client->replyDone(false);
    }

    if (!client->validSocket()) {
        disconnectClient(id);
    }
}

void
Server::pollHupHandler(size_t id) {
    Log.syserr() << "Server::POLLHUP occured on the " << _pollfds[id].fd << "socket" << Log.endl;
    if (id >= _socketsCount) {
        disconnectClient(id);
    }
}

void
Server::pollErrHandler(size_t id) {
    Log.syserr() << "Server::pollerr on [" << _pollfds[id].fd << "]" << Log.endl;
}

void
Server::finish(void) {
    _working = false;
}

void sigint_handler(int) {
    g_server->finish();
    Log << std::endl;
    Log.info() << "Server is stopping..." << Log.endl;
}

void
Server::start(void) {
    signal(SIGINT, sigint_handler);

    createSockets();
    createWorkers();

    while (isWorking()) {
        freeResponsePool();

        if (poll() <= 0) {
            continue ;
        }
    
        for (size_t id = 0; id < _pollfds.size(); id++) {
            if (_pollfds[id].revents & POLLNVAL) {
                continue;
            }

            if (id < _socketsCount && _pollfds[id].revents & POLLIN) {
                connectClient(id);
            } else if (_pollfds[id].revents & POLLIN) {
                pollInHandler(id);
            } else if (_pollfds[id].revents & POLLHUP) {
                pollHupHandler(id);
            } else if (_pollfds[id].revents & POLLOUT) {
                pollOutHandler(id);
            } else if (_pollfds[id].revents & POLLERR) {
                pollErrHandler(id);
            }
            _pollfds[id].revents = 0;
        }
    }

    destroyWorkers();
}

int
Server::poll(void) {

    int res = ::poll(_pollfds.data(), _pollfds.size(), 100000);

    if (res < 0) {
        switch (errno) {
            case EINTR: {
                if (isWorking()) {
                    Log.syserr() << "Server::poll" << Log.endl;
                }
                break ;
            }
            case EAGAIN: {
                Log.syserr() << "Server::poll" << Log.endl;
                break ;
            }
            case EINVAL: {
                struct rlimit rlim;
                getrlimit(RLIMIT_NOFILE, &rlim);
                Log.syserr() << "Server::poll" << Log.endl;
                Log.debug() << "Server:: ndfs: " << _pollfds.size() << Log.endl;
                Log.debug() << "Server:: limit(soft): " << rlim.rlim_cur << Log.endl;
                Log.debug() << "Server:: limit(hard): " << rlim.rlim_max << Log.endl;
                break ;
            }
            default:
                break ;
        }
    }
    return res;
}

static int
isFree(struct pollfd pfd) {
    return (pfd.fd == -1);
}

size_t
Server::addSocket(struct pollfd pfd, HTTP::Client *client) {

    iter_pfd it = std::find_if(_pollfds.begin(), _pollfds.end(), isFree);
    if (it == _pollfds.end()) {
        it = _pollfds.insert(_pollfds.end(), pfd);
        _clients.insert(_clients.end(), NULL);
    } else {
        *it = pfd;
    }

    size_t id = std::distance(_pollfds.begin(), it);
    _clients[id] = client;

    return id;
}

void
Server::connectClient(size_t servid) {

    struct sockaddr_in servData;
    socklen_t servLen = sizeof(servData);
    if (getsockname(_pollfds[servid].fd, (struct sockaddr *)&servData, &servLen) < 0) {
        Log.syserr() << "Server::getsockname [" << _pollfds[servid].fd << "]" << Log.endl;
        return ;
    }

    struct sockaddr_in clientData;
    socklen_t clientLen = sizeof(clientData);
    int fd = accept(_pollfds[servid].fd, (struct sockaddr *)&clientData, &clientLen);

    if (fd < 0) {
        Log.syserr() << "Server::accept" << Log.endl;
        return ;
    }

    fcntl(fd, F_SETFL, O_NONBLOCK);

    HTTP::Client *client = new HTTP::Client();
    client->setFd(fd);
    client->setPort(ntohs(clientData.sin_port));
    client->setIpAddr(inet_ntoa(clientData.sin_addr));
    client->setServerPort(ntohs(servData.sin_port));
    client->setServerIpAddr(inet_ntoa(servData.sin_addr));

    addSocket((struct pollfd){ fd, POLLIN | POLLOUT, 0 }, client);

    Log.debug() << "Server::connect [" << fd << "] -> " << client->getHostname() << Log.endl;
}

void
Server::disconnectClient(size_t id) {
    const int fd = _pollfds[id].fd;

    close(fd);
    delete _clients[id];
    _clients[id] = NULL;
    _pollfds[id].fd      = -1;
    _pollfds[id].events  = 0;
    _pollfds[id].revents = 0;
    
    Log.debug() << "Server::disconnect [" << fd << "]" << Log.endl;
}
