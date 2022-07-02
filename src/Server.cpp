#include "Server.hpp"

const std::size_t reservedClients = 64;

Server::Server() 
    : _working(true), isDaemon(false) {
    _pollfds.reserve(reservedClients);
    // _clients.reserve(reservedClients);
}

Server::~Server() {
    for (std::size_t i = 0; i < _pollfds.size(); i++) {
        if (_pollfds[i].fd != -1) {
            close(_pollfds[i].fd);
            _pollfds[i].fd = -1;
        }
    }

    // for (std::size_t i = 0; i < _clients.size(); i++) {
    //     if (_clients[i] != NULL) {
    //         delete _clients[i];
    //     }
    // }
}

// Could be used for re-reading config:
// Try to read new configuration into another server,
// and if server is not NULL, then do old_server = new_server,
// else keep old configuration and log message.
Server::Server(const Server &other) {
    operator=(other);
}

Server &
Server::operator=(const Server &other) {
    if (this != &other) {
        _servers    = other._servers;
        _pollfds    = other._pollfds;
        _clients    = other._clients;
        _sockets    = other._sockets;
    }
    return (*this);
}

bool
Server::working(void) {
    return _working;
}

Server::ServersList &
Server::operator[](std::size_t port) {
    return _servers[port];
}

Server::ServersMap &
Server::getServerBlocks(void) {
    return _servers;
}

void
Server::finish(void) {
    _working = false;
}

static void
sigint_handler(int) {
    g_server->finish();
    Log << std::endl;
    Log.info() << "Server is stopping..." << Log.endl;
}

void
Server::daemon(void) {
    pid_t pid, sid;
        
    pid = fork();
    if (pid < 0) {
        Log.syserr() << "Daemon:: fork failed" << Log.endl;
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        Log.info() << "Daemon:: starting with pid " << pid << Log.endl;
        exit(EXIT_SUCCESS);
    }

    umask(0);       
   
    sid = setsid();
    if (sid < 0) {
        Log.syserr() << "Daemon::SID creation failed" << Log.endl;
        exit(EXIT_FAILURE);
    }

    Log.logToStd(false);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void
Server::start(void) {
    if (isDaemon) {
        daemon();
    }

    signal(SIGINT, sigint_handler);

    createSockets();
    if (!working()) {
        return ;
    }
    
    startWorkers();
    while (working()) {
        addPollFd();
        if (poll() > 0) {
            process();
        }
        checkTimeout();
    }
    stopWorkers();
}

int 
Server::addListenSocket(const std::string &addr, std::size_t port) {

    IO *sock = new IO();
    if (sock == NULL) {
        Log.syserr() << "Server:: Cannot allocate memory for IO" << Log.endl;
        return -1;
    }

    if (sock->create() < 0) {
        delete sock;
        return -1;
    }

    if (sock->listen(addr, port) < 0) {
        delete sock;
        return -1;
    }

    queuePollFd(sock->rdFd(), POLLIN);
    _sockets.push_back(sock);

    return 0;
}

void
Server::createSockets(void) {

    typedef std::set<std::string> uniqueAddr;
    typedef uniqueAddr::iterator iter_ua;

    typedef std::map<std::size_t, uniqueAddr> uniqueValMap;
    typedef uniqueValMap::iterator iter_uvm;

    if (!working()) {
        return ;
    }

    uniqueValMap uniqueAddrs;
    for (iter_sm it = _servers.begin(); it != _servers.end(); ++it) {
        for (iter_sl sb = it->second.begin(); sb != it->second.end(); ++sb) {
            const std::size_t port = it->first;
            const std::string &addr = sb->getAddrRef();
            uniqueAddrs[port].insert(addr);
        }
    }

    for (iter_uvm it = uniqueAddrs.begin(); it != uniqueAddrs.end(); ++it) {
        for (iter_ua ua = it->second.begin(); ua != it->second.end(); ++ua) {
            const std::size_t port = it->first;
            const std::string &addr = *ua;
            if (addListenSocket(addr, port) < 0) {
                finish();
                return ;
            }
        }
    }
}

void
Server::startWorkers(void) {

    for (std::size_t i = 0; i < Worker::count; i++) {
        _workers[i].create();
    }
}

void
Server::stopWorkers(void) {

    for (std::size_t i = 0; i < Worker::count; i++) {
        _workers[i].join();
    }
}

void
Server::process(void) {
    for (std::size_t id = 0; id < _pollfds.size(); id++) {
        const int fd = _pollfds[id].fd;

        if (fd < 0 || _pollfds[id].revents & POLLNVAL) {
            continue ;
        }

        if (id < _sockets.size()) {
            if (_pollfds[id].revents & POLLIN) {
                connect(id, fd);
            }
        } else {

            if (_pollfds[id].revents & POLLIN) {
                _clients[fd]->pollin(fd);
            } else if (_pollfds[id].revents & POLLHUP) {
                _clients[fd]->pollhup(fd);
            } else if (_pollfds[id].revents & POLLOUT) {
                _clients[fd]->pollout(fd);
            } else if (_pollfds[id].revents & POLLERR) {
                _clients[fd]->pollerr(fd);
            }

            if (_clients[fd]->shouldBeClosed()) {
                disconnect(fd);
            }
        }
        _pollfds[id].revents = 0;
    }
}

int
Server::poll(void) {

    int res = ::poll(_pollfds.data(), _pollfds.size(), 100000);

    if (res < 0) {
        if (working()) {
            Log.syserr() << "Server::poll" << Log.endl;
        }
    }
    return res;
}


static int
isFree(struct pollfd pfd) {
    return (pfd.fd == -1);
}

void
Server::queuePollFd(int fd, int events) {

    struct pollfd tmp = { fd, events, 0 };
    _waitingFds.push_back(tmp);
}

void
Server::addPollFd(void) {

    do {
        struct pollfd tmp;
        try {
            tmp = _waitingFds.pop_front();
        } catch (Pool<struct pollfd>::Empty &e) {
            return ;
        }

        iter_pfd it = std::find_if(_pollfds.begin(), _pollfds.end(), isFree);
        if (it == _pollfds.end()) {
            it = _pollfds.insert(_pollfds.end(), tmp);
        } else {
            *it = tmp;
        }
    } while (!_waitingFds.empty());

    return ;
}

void
Server::rmPollFd(int fd) {

    Log.debug() << "Server:: [" << fd << "]" << " rmPollfd" << Log.endl;
    if (fd < 0) {
        return ;
    }

    for (iter_pfd it = _pollfds.begin(); it != _pollfds.end(); ++it) {
        if (it->fd == fd) {
            it->fd = -1;
            it->events = 0;
            it->revents = 0;
            break ;
        }   
    }
}

void
Server::addClient(int fd, HTTP::Client *client) {
    _clients[fd] = client;
}

void
Server::checkTimeout(void) {

    std::time_t cur = std::time(0);

    for (ClientsMap::iterator it = _clients.begin(); it != _clients.end(); ++it) {

        if (it->second->shouldBeClosed()) {
            continue ;
        }

        std::time_t t_client = it->second->getClientTimeout();
        std::time_t t_target = it->second->getTargetTimeout();

        if (t_client != 0 && cur - t_client > MAX_CLIENT_TIMEOUT) {
            Log.debug() << "Server:: [" << it->second->getClientIO()->rdFd() << "] client timeout exceeded" << Log.endl;
            it->second->shouldBeClosed(true);
        }

        if (t_target != 0 && cur - t_target > MAX_TARGET_TIMEOUT) {
            Log.debug() << "Server:: [" << it->second->getTargetIO()->rdFd() << "] target timeout exceeded" << Log.endl;
            // need to kill child process
            // close TARGET socket
            // remove TARGET link
            // set response status to gateway timeout
        }
    }
}

void
Server::connect(std::size_t servid, int servfd) {

    struct sockaddr_in servData;
    socklen_t servLen = sizeof(servData);
    if (getsockname(servfd, (struct sockaddr *)&servData, &servLen) < 0) {
        Log.syserr() << "Server::getsockname [" << servfd << "]" << Log.endl;
        return ;
    }

    struct sockaddr_in clientData;
    socklen_t clientLen = sizeof(clientData);
    int fd = accept(servfd, (struct sockaddr *)&clientData, &clientLen);

    if (fd < 0) {
        Log.syserr() << "Server::accept" << Log.endl;
        return ;
    }

    HTTP::Client *client = new HTTP::Client();
    if (client == NULL) {
        Log.syserr() << "Server::Cannot allocate memory for Client" << Log.endl;
        close(fd);
        return ;
    }

    client->setServerIO(_sockets[servid]);
    client->setClientTimeout(std::time(0));
    client->getClientIO()->setFd(fd);
    client->getClientIO()->nonblock();
    client->getClientIO()->setAddr(inet_ntoa(clientData.sin_addr));
    client->getClientIO()->setPort(ntohs(clientData.sin_port));

    queuePollFd(fd, POLLIN | POLLOUT);

    _clients[fd] = client;

    Log.debug() << "Server::connect [" << fd << "] -> " << client->getHostname() << Log.endl;
}

void
Server::disconnect(int fd) {

    if (fd < 0) {
        return ;
    }

    ClientsMap::iterator it = _clients.find(fd);
    if (it == _clients.end()) {
        return ;
    }

    HTTP::Client *client = it->second;
    if (client == NULL) {
        return ;
    }

    Log.debug() << "Server:: [" << client->getClientIO()->rdFd() << "] disconnect" << Log.endl;
    
    rmPollFd(client->getClientIO()->rdFd());
    rmPollFd(client->getTargetIO()->rdFd());

    _clients.erase(client->getClientIO()->rdFd());
    _clients.erase(client->getTargetIO()->rdFd());

    delete client;
    
}
