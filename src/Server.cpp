#include "Server.hpp"

const std::size_t reservedClients = 64;

Server::Server()
    : _working(true), isDaemon(false) {
    _pollfds.reserve(reservedClients);

    pthread_mutex_init(&_m_new_resp, NULL);
    pthread_mutex_init(&_m_new_pfds, NULL);
    pthread_mutex_init(&_m_new_clnt, NULL);
    pthread_mutex_init(&_m_del_pfds, NULL);
    pthread_mutex_init(&_m_del_clnt, NULL);
}

Server::~Server(void) {

    std::set<HTTP::Client *> uniqueClients;
    for (iter_cm it = _clients.begin(); it != _clients.end(); ++it) {
        uniqueClients.insert(it->second);
    }

    std::set<HTTP::Client *>::iterator it;
    for (it = uniqueClients.begin(); it != uniqueClients.end(); ++it) {
        delete *it;
    }

    for (size_t i = 0; i < _sockets.size(); ++i) {
        delete _sockets[i];
    }

    pthread_mutex_destroy(&_m_new_resp);
    pthread_mutex_destroy(&_m_new_pfds);
    pthread_mutex_destroy(&_m_new_clnt);
    pthread_mutex_destroy(&_m_del_pfds);
    pthread_mutex_destroy(&_m_del_clnt);
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

void Server::daemonMode(void) {
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) {
        Log.syserr() << "Server::Daemon: fork failed" << Log.endl;
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        Log.info() << "Server::Daemon: starting with pid " << pid << Log.endl;
        exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();
    if (sid < 0) {
        Log.syserr() << "Server::Daemon: SID creation failed" << Log.endl;
        exit(EXIT_FAILURE);
    }

    Log.logToStd(false);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void Server::start(void) {
    if (isDaemon) {
        daemonMode();
    }

    signal(SIGINT, sigint_handler);

    createSockets();
    if (!working()) {
        return;
    }

    startWorkers();
    while (working()) {
        emptyNewClientQ();
        emptyNewFdsQ();
    
        if (poll() > 0) {
            process();
        }
        checkTimeout();
    
        emptyDelFdsQ();
        emptyDelClientQ();
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
    _pollfds.push_back((struct pollfd){sock->rdFd(), POLLIN, 0});
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
            continue ;
        }

        if (id < _sockets.size()) {
            if (_pollfds[id].revents & POLLIN) {
                connect(id, fd);
            }
        } else if (_clients[fd] != NULL) {
            if (_pollfds[id].revents & POLLIN) {
                _clients[fd]->pollin(fd);
            } else if (_pollfds[id].revents & POLLHUP) {
                _clients[fd]->pollhup(fd);
            } else if (_pollfds[id].revents & POLLOUT) {
                _clients[fd]->pollout(fd);
            } else if (_pollfds[id].revents & POLLERR) {
                _clients[fd]->pollerr(fd);
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

void Server::checkTimeout(void) {
    std::time_t cur = Time::now();

    for (ClientsMap::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        HTTP::Client *client = it->second;

        if (client == NULL) {
            continue;
        }

        if (client->shouldBeRemoved()) {
            // If client should be closed we MUST wait until reply is done
            addToDelFdsQ(client->getClientIO()->rdFd());
            addToDelFdsQ(client->getGatewayIO()->rdFd());
            addToDelFdsQ(client->getGatewayIO()->wrFd());
            continue ;
        }

        std::time_t t_client = client->getClientTimeout();
        if (t_client != 0 && cur - t_client > MAX_CLIENT_TIMEOUT) {
            addToDelFdsQ(client->getClientIO()->rdFd());
            addToDelFdsQ(client->getGatewayIO()->rdFd());
            addToDelFdsQ(client->getGatewayIO()->wrFd());
            Log.debug() << "Server:: [" << client->getClientIO()->rdFd() << "] client timeout exceeded" << Log.endl;
        }

        std::time_t t_gateway = client->getGatewayTimeout();
        if (t_gateway != 0 && cur - t_gateway > MAX_GATEWAY_TIMEOUT) {
            addToDelFdsQ(client->getGatewayIO()->rdFd());
            addToDelFdsQ(client->getGatewayIO()->wrFd());
            Log.debug() << "Server:: [" << client->getGatewayIO()->rdFd() << "] gateway timeout exceeded" << Log.endl;
            // need to kill child process if CGI
            // close only GATEWAY socket
            // set response status to gateway timeout and reply to client
        }

        // Add comparison with MAX_SESSION_TIMEOUT
    }
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
    
    int fd = accept(servfd, (struct sockaddr *)&clientData, &clientLen);
    if (fd < 0) {
        Log.syserr() << "Server::accept" << Log.endl;
        return;
    }

    HTTP::Client *client = new HTTP::Client();
    if (client == NULL) {
        Log.syserr() << "Server::Cannot allocate memory for Client" << Log.endl;
        close(fd);
        return ;
    }

    client->setServerIO(_sockets[servid]);
    client->setClientTimeout(Time::now());
    client->getClientIO()->setFd(fd);
    client->getClientIO()->nonblock();
    client->getClientIO()->setAddr(inet_ntoa(clientData.sin_addr));
    client->getClientIO()->setPort(ntohs(clientData.sin_port));

    addToNewClientQ(fd, client);
    addToNewFdsQ(fd);

    Log.debug() << "Server::connect [" << fd << "] -> " << client->getHostname() << Log.endl;
}

// The functions below used to work with different queues

void Server::addToNewFdsQ(int fd) {
    pthread_mutex_lock(&_m_new_pfds);
    if (fd >= 0) {
        Log.debug() << "Server::addToNewFdsQ [" << fd << "]" << Log.endl;
        _q_newPfds.push(fd);
    }
    pthread_mutex_unlock(&_m_new_pfds);
}

void Server::addToDelFdsQ(int fd) {
    pthread_mutex_lock(&_m_del_pfds);
    if (fd >= 0) {
        std::pair<std::set<int>::iterator, bool> p = _q_delPfds.insert(fd);
        if (p.second) {
            Log.debug() << "Server::addToDelFdsQ [" << fd << "]" << Log.endl;
        }
    }
    pthread_mutex_unlock(&_m_del_pfds);
}

void Server::addToNewClientQ(int fd, HTTP::Client *client) {
    pthread_mutex_lock(&_m_new_clnt);
    Log.debug() << "Server::addToNewClientQ [" << fd << "]" << Log.endl;
    _q_newClients.push(std::make_pair(fd, client));
    pthread_mutex_unlock(&_m_new_clnt);
}

void Server::addToDelClientQ(HTTP::Client *client) {
    pthread_mutex_lock(&_m_del_clnt);
    if (client != NULL) {
        std::pair<std::set<HTTP::Client *>::iterator, bool> p = _q_delClients.insert(client);
        if (p.second) {
            Log.debug() << "Server::addToDelClientQ [" << client->getClientIO()->rdFd() << "]" << Log.endl;
        }
    }
    pthread_mutex_unlock(&_m_del_clnt);
}

void Server::emptyNewFdsQ(void) {
    pthread_mutex_lock(&_m_new_pfds);

    while (_q_newPfds.size() > 0) {
        int tmpfd = _q_newPfds.front();
        _q_newPfds.pop();

        struct pollfd tmp = { tmpfd, POLLIN | POLLOUT, 0 };

        iter_pfd it = std::find_if(_pollfds.begin(), _pollfds.end(), isFree);
        if (it == _pollfds.end()) {
            _pollfds.push_back(tmp);
        } else {
            *it = tmp;
        }

        Log.debug() << "Server::emptyNewFdsQ [" << tmp.fd << "]" << Log.endl;
    }

    pthread_mutex_unlock(&_m_new_pfds);
}

void Server::emptyDelFdsQ(void) {
    pthread_mutex_lock(&_m_del_pfds);

    while (_q_delPfds.size() > 0) {

        std::set<int>::iterator beg = _q_delPfds.begin();
        int tmpfd = *beg;
        _q_delPfds.erase(beg);

        struct pollfd tmp = { -1, 0, 0 };

        for (size_t i = 0; i < _pollfds.size(); ++i) {
            if (_pollfds[i].fd == tmpfd) {
                _pollfds[i] = tmp;
                break ;
            }
        }

        pthread_mutex_lock(&_m_new_clnt);
        iter_cm it = _clients.find(tmpfd);
        if (it != _clients.end()) {
            addToDelClientQ(it->second);
        }
        pthread_mutex_unlock(&_m_new_clnt);

        Log.debug() << "Server::emptyDelFdsQ [" << tmp.fd << "]" << Log.endl;
    }

    pthread_mutex_unlock(&_m_del_pfds);
}

void Server::emptyNewClientQ(void) {
    pthread_mutex_lock(&_m_new_clnt);

    while (_q_newClients.size() > 0) {
        std::pair<int, HTTP::Client *> tmp = _q_newClients.front();
        _q_newClients.pop();

        _clients[tmp.first] = tmp.second;
    
        Log.debug() << "Server::emptyNewClientQ [" << tmp.first << "]" << Log.endl;
    }

    pthread_mutex_unlock(&_m_new_clnt);
}

void Server::emptyDelClientQ(void) {
    pthread_mutex_lock(&_m_del_clnt);

    while (_q_delClients.size() > 0) {

        std::set<HTTP::Client *>::iterator beg = _q_delClients.begin();
        HTTP::Client *tmp = *beg;
        _q_delClients.erase(beg);

        pthread_mutex_lock(&_m_new_resp);
        
        std::list<HTTP::Response *>::iterator it;
        for (it = _q_newResponses.begin(); it != _q_newResponses.end(); ++it) {
            if ((*it) != NULL && (*it)->getClient() == tmp) {
                *it = NULL;
            }
        }

        pthread_mutex_unlock(&_m_new_resp);

        int cio_fd = -1;
        int tio_fdr = -1;
        int tio_fdw = -1;

        if (tmp->getClientIO()) {
            cio_fd = tmp->getClientIO()->rdFd();
        }

        if (tmp->getGatewayIO()) {
            tio_fdr = tmp->getGatewayIO()->rdFd();
            tio_fdw = tmp->getGatewayIO()->wrFd();
        }

        if (cio_fd != -1) {
            _clients[cio_fd] = NULL;
        }

        if (tio_fdr != -1) {
            _clients[tio_fdr] = NULL;
        }

        if (tio_fdw != -1 && tio_fdw != tio_fdr) {
            _clients[tio_fdw] = NULL;
        }

        delete tmp;

        Log.debug() << "Server::emptyNewClientQ [" << cio_fd << "]" << Log.endl;
    }

    pthread_mutex_unlock(&_m_del_clnt);
}


void Server::addToRespQ(HTTP::Response *res) {
    pthread_mutex_lock(&_m_new_resp);
    _q_newResponses.push_back(res);
    pthread_mutex_unlock(&_m_new_resp);
}

HTTP::Response *Server::rmFromRespQ(void) {
    HTTP::Response *res = NULL;

    pthread_mutex_lock(&_m_new_resp);

    if (_q_newResponses.size() > 0) {
        res = _q_newResponses.front();
        _q_newResponses.pop_front();
    }

    pthread_mutex_unlock(&_m_new_resp);

    return res;
}


