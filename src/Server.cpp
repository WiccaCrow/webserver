#include "Server.hpp"

const std::size_t reservedClients = 64;

Server::Server()
    : _working(true), isDaemon(false) {
    _pollfds.reserve(reservedClients);
    // _clients.reserve(reservedClients);

    pthread_mutex_init(&_fds_lock, NULL);
    pthread_mutex_init(&_res_lock, NULL);
    pthread_mutex_init(&_cln_lock, NULL);
}

Server::~Server() {
    // for (std::size_t i = 0; i < _pollfds.size(); i++) {
    //     if (_pollfds[i].fd != -1) {
    //         _pollfds[i].fd = -1;
    //     }
    // }

    for (size_t i = 0; i < _sockets.size(); ++i) {
        delete _sockets[i];
        _sockets[i] = NULL;
    }

    std::set<HTTP::Client *> uniqueClients;
    for (iter_cm it = _clients.begin(); it != _clients.end(); ++it) {
        uniqueClients.insert(it->second);
    }

    std::set<HTTP::Client *>::iterator it = uniqueClients.begin();
    for (; it != uniqueClients.end(); ++it) {
        delete *it;
    }

    pthread_mutex_destroy(&_fds_lock);
    pthread_mutex_destroy(&_res_lock);
    pthread_mutex_destroy(&_cln_lock);
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
        _servers = other._servers;
        _pollfds = other._pollfds;
        _clients = other._clients;
        _sockets = other._sockets;
    }
    return (*this);
}

bool Server::working(void) {
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

void Server::finish(void) {
    _working = false;
}

static void
sigint_handler(int) {
    Log << std::endl;
    Log.info() << "Server is stopping..." << Log.endl;
    g_server->finish();
}

void Server::daemon(void) {
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

void Server::start(void) {
    if (isDaemon) {
        daemon();
    }

    signal(SIGINT, sigint_handler);

    createSockets();
    if (!working()) {
        return;
    }

    startWorkers();
    while (working()) {
        emptyFdsQueue();
        if (poll() > 0) {
            process();
        }
        checkTimeout();
    }
    stopWorkers();
}

int Server::addListenSocket(const std::string &addr, std::size_t port) {
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
    addToQueue((struct pollfd){sock->rdFd(), POLLIN, 0});
    _sockets.push_back(sock);

    return 0;
}

void Server::createSockets(void) {
    typedef std::set<std::string> uniqueAddr;
    typedef uniqueAddr::iterator  iter_ua;

    typedef std::map<std::size_t, uniqueAddr> uniqueValMap;
    typedef uniqueValMap::iterator            iter_uvm;

    if (!working()) {
        return;
    }

    uniqueValMap uniqueAddrs;
    for (iter_sm it = _servers.begin(); it != _servers.end(); ++it) {
        for (iter_sl sb = it->second.begin(); sb != it->second.end(); ++sb) {
            const std::size_t  port = it->first;
            const std::string &addr = sb->getAddrRef();
            uniqueAddrs[port].insert(addr);
        }
    }

    for (iter_uvm it = uniqueAddrs.begin(); it != uniqueAddrs.end(); ++it) {
        for (iter_ua ua = it->second.begin(); ua != it->second.end(); ++ua) {
            const std::size_t  port = it->first;
            const std::string &addr = *ua;
            if (addListenSocket(addr, port) < 0) {
                finish();
                return;
            }
        }
    }
}

void Server::startWorkers(void) {
    for (std::size_t i = 0; i < Worker::count; i++) {
        _workers[i].create();
    }
}

void Server::stopWorkers(void) {
    for (std::size_t i = 0; i < Worker::count; i++) {
        _workers[i].join();
    }
}

void Server::process(void) {
    for (std::size_t id = 0; id < _pollfds.size(); id++) {
        const int fd = _pollfds[id].fd;

        if (fd < 0 || _pollfds[id].revents & POLLNVAL) {
            continue;
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

            if (_clients[fd]->shouldBeRemoved()) {
                disconnect(fd);
            }
        }
        _pollfds[id].revents = 0;
    }
}

int Server::poll(void) {
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

void Server::addToQueue(struct pollfd pfd) {
    pthread_mutex_lock(&_fds_lock);
    Log.debug() << "Server::addToQueue pfd " << pfd.fd << Log.endl;
    _pendingFds.push(pfd);
    pthread_mutex_unlock(&_fds_lock);
}

void Server::emptyFdsQueue(void) {
    pthread_mutex_lock(&_fds_lock);

    while (_pendingFds.size() > 0) {
        struct pollfd tmp = _pendingFds.front();
        _pendingFds.pop();

        iter_pfd it = std::find_if(_pollfds.begin(), _pollfds.end(), isFree);
        if (it == _pollfds.end()) {
            it = _pollfds.insert(_pollfds.end(), tmp);
        } else {
            *it = tmp;
        }

        Log.debug() << "Server::emptyFdsQueue pfd " << tmp.fd << Log.endl;
    }

    pthread_mutex_unlock(&_fds_lock);
}

void Server::rmPollFd(int fd) {
    Log.debug() << "Server:: [" << fd << "]"
                << " rmPollfd" << Log.endl;
    if (fd < 0) {
        return;
    }

    for (iter_pfd it = _pollfds.begin(); it != _pollfds.end(); ++it) {
        if (it->fd == fd) {
            it->fd = -1;
            it->events = 0;
            it->revents = 0;
            break;
        }
    }
}

void Server::addToQueue(HTTP::Response *res) {
    pthread_mutex_lock(&_res_lock);
    _pendingResps.push(res);
    pthread_mutex_unlock(&_res_lock);
}

HTTP::Response *Server::rmFromQueue(void) {
    HTTP::Response *res = NULL;

    pthread_mutex_lock(&_res_lock);

    if (_pendingResps.size() > 0) {
        res = _pendingResps.front();
        _pendingResps.pop();
    }

    pthread_mutex_unlock(&_res_lock);

    return res;
}

void Server::addClient(int fd, HTTP::Client *client) {
    pthread_mutex_lock(&_cln_lock);
    Log.debug() << "Server::addClient " << fd << " " << client << Log.endl;
    _clients[fd] = client;
    pthread_mutex_unlock(&_cln_lock);
}

void Server::checkTimeout(void) {
    std::time_t cur = std::time(0);

    pthread_mutex_lock(&_cln_lock);
    for (ClientsMap::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->shouldBeClosed()) {
            continue;
        }

        std::time_t t_client = it->second->getClientTimeout();
        std::time_t t_target = it->second->getTargetTimeout();

        if (t_client != 0 && cur - t_client > MAX_CLIENT_TIMEOUT) {
            Log.debug() << "Server:: [" << it->second->getClientIO()->rdFd() << "] client timeout exceeded" << Log.endl;
            it->second->shouldBeRemoved(true);
        }

        if (t_target != 0 && cur - t_target > MAX_TARGET_TIMEOUT) {
            Log.debug() << "Server:: [" << it->second->getTargetIO()->rdFd() << "] target timeout exceeded" << Log.endl;
            // need to kill child process
            // close TARGET socket
            // remove TARGET link
            // set response status to gateway timeout
        }
    }
    pthread_mutex_unlock(&_cln_lock);
}

void Server::connect(std::size_t servid, int servfd) {
    struct sockaddr_in servData;
    socklen_t          servLen = sizeof(servData);
    if (getsockname(servfd, (struct sockaddr *)&servData, &servLen) < 0) {
        Log.syserr() << "Server::getsockname [" << servfd << "]" << Log.endl;
        return;
    }

    struct sockaddr_in clientData;
    socklen_t          clientLen = sizeof(clientData);
    int                fd = accept(servfd, (struct sockaddr *)&clientData, &clientLen);

    if (fd < 0) {
        Log.syserr() << "Server::accept" << Log.endl;
        return;
    }

    HTTP::Client *client = new HTTP::Client();
    if (client == NULL) {
        Log.syserr() << "Server::Cannot allocate memory for Client" << Log.endl;
        close(fd);
        return;
    }

    client->setServerIO(_sockets[servid]);
    client->setClientTimeout(std::time(0));
    client->getClientIO()->setFd(fd);
    client->getClientIO()->nonblock();
    client->getClientIO()->setAddr(inet_ntoa(clientData.sin_addr));
    client->getClientIO()->setPort(ntohs(clientData.sin_port));

    addToQueue((struct pollfd){fd, POLLIN | POLLOUT, 0});
    addClient(fd, client);

    Log.debug() << "Server::connect [" << fd << "] -> " << client->getHostname() << Log.endl;
}

void Server::disconnect(int fd) {
    if (fd < 0) {
        return;
    }

    ClientsMap::iterator it = _clients.find(fd);
    if (it == _clients.end()) {
        return;
    }

    HTTP::Client *client = it->second;
    if (client == NULL) {
        return;
    }

    IO *cio = client->getClientIO();
    IO *tio = client->getTargetIO();

    Log.debug() << "Server:: [" << client->getClientIO()->rdFd() << "] disconnect" << Log.endl;

    if (cio) {
        rmPollFd(cio->rdFd());
        _clients.erase(cio->rdFd());
    }
    if (tio) {
        rmPollFd(tio->rdFd());
        _clients.erase(tio->rdFd());
    }

    delete client;
}
