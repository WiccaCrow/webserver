#include "Server.hpp"
#include "Pool.hpp"

const size_t reservedClients = 64;

// Declared in Globals.hpp
// Designed to stop workers
bool finished;

// Thread-safe container
typedef Pool<HTTP::Request *>  RequestPool;
typedef Pool<HTTP::Response *> ResponsePool;

RequestPool requests;
ResponsePool responses;

Server::Server() 
    : _socketsCount(0) {
    _pollfds.reserve(reservedClients);
    _clients.reserve(reservedClients);

    for (size_t i = 0; i < WORKERS; i++) {
        workerInfos[i].id = i;
    }
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
        Log.syserr() << "Server::socket ->" << addr << ":" << port << Log.endl;
        exit(1);
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
        Log.syserr() << "Server::setsockopt ->" << addr << ":" << port << Log.endl;
        exit(1);
    }
    if (bind(fd, (struct sockaddr *)&data, sizeof(data)) < 0) {
        Log.syserr() << "Server::bind ->" << addr << ":" << port << Log.endl;
        exit(1);
    }
    if (listen(fd, SOMAXCONN) < 0) {
        Log.syserr() << "Server::listen ->" << addr << ":" << port << Log.endl;
        exit(1);
    }
    
    return fd;
}

void *workerCycle(void *ptr) {

    WorkerInfo *w = reinterpret_cast<WorkerInfo *>(ptr);

    while (!finished) {
    
        HTTP::Request *req = NULL;
    
        try {
            req = requests.pop_front();

        } catch (RequestPool::Empty &e) {            
            usleep(WORKER_TIMEOUT);
            continue ;
        }

        HTTP::Response *res = new HTTP::Response(req);

        if (res == NULL) {
            Log.syserr() << "Cannot allocate memory for Response" << Log.endl;
            // Not sure what to do; only continue will lead to leaks
            continue ;
        }

        const int fd = res->getClient()->getFd();
        const std::string &uri = req->getRawUri();

        Log.debug() << "Worker[" << w->id << "] for [" << fd << "]: req " << uri << " started" << Log.endl; 
        res->handle();
        Log.debug() << "Worker[" << w->id << "] for [" << fd << "]: req " << uri << " processed" << Log.endl; 
        responses.push_back(res);
        Log.debug() << "Worker[" << w->id << "] for [" << fd << "]: req " << uri << " returned" << Log.endl; 
 
    }
    return NULL;
}

void
Server::createWorkers(void) {

    finished = false;
    for (size_t i = 0; i < WORKERS; i++) {
        if (pthread_create(&_threads[i], NULL, workerCycle, &workerInfos[i])) {
            Log.syserr() << "Server::pthread create failed for worker" << workerInfos[i].id << Log.endl;
        } else {
            Log.debug() << "Server:: Worker[" << workerInfos[i].id << "] created" << Log.endl;
        }
    }
}

void
Server::destroyWorkers(void) {
    
    finished = true;
    for (size_t i = 0; i < WORKERS; i++) {
        if (pthread_join(_threads[i], NULL)) {
            Log.syserr() << "Server::pthread join failed for worker" << workerInfos[i].id << Log.endl;
        } else {
            Log.debug() << "Server:: Worker[" << workerInfos[i].id << "] destroyed" << Log.endl;
        }
    }
}

void
Server::freePool(void) {
    
    HTTP::Response *res = NULL;

    do {
        try {
            res = responses.pop_front();
            res->getClient()->addResponse(res);
            Log.debug() << "Server::freePool into [" << res->getClient()->getFd() << "]: " << res->getRequest()->getRawUri() << Log.endl;
        } catch (ResponsePool::Empty &e) {            
            break;
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
            _pollfds.push_back((struct pollfd) { fd, POLLIN, 0 });
            _clients.push_back(NULL);
            _socketsCount++;
            Log.info() << "Server::listen [" << fd << "] -> " << *addr << ":" << port << Log.endl;
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

void
Server::pollHupHandler(size_t id) {
    Log.syserr() << "Server::POLLHUP occured on the " << _pollfds[id].fd << "socket" << Log.endl;
    if (id >= _socketsCount) {
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
Server::pollErrHandler(size_t id) {
    Log.syserr() << "Server::pollerr on [" << _pollfds[id].fd << "]" << Log.endl;
    exit(1);
}

void
Server::start(void) {
    fillServBlocksFds();
    createWorkers();
    while (1) {
        freePool();
        _pollResult = 0;

        pollServ();
        for (size_t id = 0; id < _pollfds.size(); id++) {
            if (id < _socketsCount &&_pollfds[id].revents & POLLIN) {
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

void
Server::handlePollError() {
    Log.syserr() << "Server::poll" << Log.endl;

    if (errno == EINVAL) {
        struct rlimit rlim;
        getrlimit(RLIMIT_NOFILE, &rlim);
        Log.debug() << "ndfs: " << _pollfds.size() << Log.endl;
        Log.debug() << "limit(soft): " << rlim.rlim_cur << Log.endl;
        Log.debug() << "limit(hard): " << rlim.rlim_max << Log.endl;
    }
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
isFree(struct pollfd pfd) {
    return (pfd.fd == -1);
}

size_t
Server::addClient(int fd, HTTP::Client *client) {
    std::vector<struct pollfd>::iterator it;
    
    struct pollfd tmp = {
        fd, POLLIN | POLLOUT, 0
    };

    it = std::find_if(_pollfds.begin(), _pollfds.end(), isFree);
    if (it == _pollfds.end()) {
        it = _pollfds.insert(_pollfds.end(), tmp);
        _clients.insert(_clients.end(), NULL);
    } else {
        *it = tmp;
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

    size_t id = addClient(fd, client);

    Log.debug() << "Server::connect [" << fd << "] -> " << id << " " << client->getHostname() << Log.endl;
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
