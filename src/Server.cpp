#include "Server.hpp"

const std::size_t reservedClients = 64;

Server::Server()
    : _working(true)
    , isDaemon(false) {
    _pollfds.reserve(reservedClients);

    pthread_mutex_init(&_m_new_resp, NULL);
    pthread_mutex_init(&_m_new_pfds, NULL);
    pthread_mutex_init(&_m_new_clnt, NULL);
    pthread_mutex_init(&_m_del_pfds, NULL);
    pthread_mutex_init(&_m_del_clnt, NULL);
    pthread_mutex_init(&_m_link, NULL);
    pthread_mutex_init(&_m_sessions, NULL);

    HTTP::ETag::StaticConstructor();
}

Server::~Server(void) {

    for (iter_cv it = _clients.begin(); it != _clients.end(); ++it) {
        if (*it != NULL) {
            delete *it;
        }
    }

    for (size_t i = 0; i < _sockets.size(); ++i) {
        delete _sockets[i];
    }

    pthread_mutex_destroy(&_m_new_resp);
    pthread_mutex_destroy(&_m_new_pfds);
    pthread_mutex_destroy(&_m_new_clnt);
    pthread_mutex_destroy(&_m_del_pfds);
    pthread_mutex_destroy(&_m_del_clnt);
    pthread_mutex_destroy(&_m_link);
    pthread_mutex_destroy(&_m_sessions);

    HTTP::ETag::StaticDestructor();
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
    std::cout << std::endl;
    Log.info() << "Server is stopping..." << Log.endl;
    g_server->finish();
}

bool
Server::isServerHostname(const std::string &name) {
    return (_hostnames.find(name) != _hostnames.end());
}

int
Server::initHostnamesSet(void) {

    char str[1024] = {0};
    if (gethostname(str, 1024) < 0) {
        Log.syserr() << "Server:: gethostname failed" << Log.endl;
        finish();
        return 0;
    }

    hostent *host = gethostbyname(str);
    if (host == NULL) {
        Log.syserr() << "Server:: gethostbyname failed" << Log.endl;
        finish();
        return 0;
    }

    _hostnames.insert(host->h_name);
    for (size_t i = 0; host->h_aliases[i]; i++) {
        _hostnames.insert(host->h_aliases[i]);
    }

    for (size_t i = 0; host->h_addr_list[i]; i++)
    {
        char *ip = inet_ntoa(*(in_addr *)host->h_addr_list[i]);
        if (ip != NULL) {
            _hostnames.insert(ip);
        }
        hostent *h = gethostbyaddr((in_addr *)host->h_addr_list[i], host->h_length, host->h_addrtype);
        if (h != NULL) {
            _hostnames.insert(h->h_name);
            
            for (size_t j = 0; h->h_aliases[j]; j++) {
                _hostnames.insert(h->h_aliases[j]);
            }
        }
    }

    for (std::set<std::string>::iterator it = _hostnames.begin(); it != _hostnames.end(); it++) {
        Log.debug() << *it << Log.endl;
    }

    return 1;
}

#ifdef WS_DAEMON_MODE

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

#else

void
Server::daemonMode(void) {
    Log.error() << "This version compiled without daemon mode flag." << Log.endl;
    Log.error() << "To run server as daemon, recompile it with WS_DAEMON_MODE." << Log.endl;
}

#endif

void Server::start(void) {

    if (isDaemon) {
        daemonMode();
    }

    signal(SIGINT, sigint_handler);

    initHostnamesSet();
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
        checkSessionsTimeout();
    
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

    if (sock->socket() < 0) {
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

    _workers = new Worker[settings.workers];
    
    if (_workers == NULL) {
        Log.syserr() << "Cannot allocate memory for workers" << Log.endl;
        return;
    }

    for (std::size_t i = 0; i < Worker::count; i++) {
        _workers[i].create();
    }
}

void Server::stopWorkers(void) {

    for (std::size_t i = 0; i < Worker::count; i++) {
        _workers[i].join();
    }

    delete[] _workers;
    _workers = NULL;
}

void
Server::pollin(int fd) {

    // Log.debug() << "Server::pollin [" << fd << "]" << Log.endl;

    size_t id = _connector[fd];

    if (id < 0 || id >= _clients.size()) {
        Log.debug() << "Server::pollin:: invalid id: " << fd << " " << id << Log.endl;
        return ;
    }

    HTTP::Client *client = _clients[id];
    if (client == NULL) {
        return ;
    }

    if (fd == client->getClientIO()->rdFd()) {
        client->tryReceiveRequest(fd);

    } else if (fd == client->getGatewayIO()->rdFd()) {
        client->tryReceiveResponse(fd);
    }
}

void
Server::pollout(int fd) {

    // Log.debug() << "Server::pollout [" << fd << "]" << Log.endl;

    size_t id = _connector[fd];

    if (id < 0 || id >= _clients.size()) {
        Log.debug() << "Server::pollout:: invalid id: " << fd << " " << id << Log.endl;
        return ;
    }

    HTTP::Client *client = _clients[id];
    if (client == NULL) {
        return ;
    }

    if (fd == client->getClientIO()->wrFd()) {
        client->tryReplyResponse(fd);

    } else if (fd == client->getGatewayIO()->wrFd()) {
        client->tryReplyRequest(fd); 
    }
}

void
Server::pollhup(int fd) {

    Log.syserr() << "Server::pollhup [" << fd << "]" << Log.endl;

    size_t id = _connector[fd];

    if (id < 0 || id >= _clients.size()) {
        Log.debug() << "Server::pollhup:: invalid id: " << id << Log.endl;
    }

    HTTP::Client *client = _clients[id];
    if (client == NULL) {
        return ;
    }

    if (fd == client->getClientIO()->rdFd()) {
        // client->shouldBeRemoved(true);
        unlink(fd);

    } else if (fd == client->getGatewayIO()->wrFd()) {
        unlink(fd);
        // addToDelFdsQ(client->getGatewayIO()->wrFd());
        // client->getGatewayIO()->closeWrFd();

    } else if (fd == client->getGatewayIO()->rdFd()) {
        client->tryReceiveResponse(fd);
        unlink(fd);
        // addToDelFdsQ(client->getGatewayIO()->rdFd());
        // client->getGatewayIO()->closeRdFd();
    }    
}

void
Server::pollerr(int fd) {
    Log.syserr() << "Server::pollerr [" << fd << "]" << Log.endl;

    size_t id = _connector[fd];

    if (id < 0 || id >= _clients.size()) {
        Log.debug() << "Server::pollerr:: invalid id: " << id << Log.endl;
    }

    HTTP::Client *client = _clients[id];
    if (client == NULL) {
        return ;
    }

    if (fd == client->getClientIO()->rdFd()) {
        // client->shouldBeRemoved(true);
        unlink(fd);

    } else if (fd == client->getGatewayIO()->wrFd()) {
        unlink(fd);
        // addToDelFdsQ(client->getGatewayIO()->wrFd());
        // client->getGatewayIO()->closeWrFd();

    } else if (fd == client->getGatewayIO()->rdFd()) {
        unlink(fd);
        // addToDelFdsQ(client->getGatewayIO()->rdFd());
        // client->getGatewayIO()->closeRdFd();
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
        } else {
            if (_pollfds[id].revents & POLLIN) {
                pollin(fd);
            } else if (_pollfds[id].revents & POLLHUP) {
                pollhup(fd);
            } else if (_pollfds[id].revents & POLLOUT) {
                pollout(fd);
            } else if (_pollfds[id].revents & POLLERR) {
                pollerr(fd);
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
isFdFree(struct pollfd pfd) {
    return (pfd.fd == -1);
}

static int
isClientFree(HTTP::Client *client) {
    return (client == NULL);
}

void
Server::checkTimeout(void) {

    for (ClientsVec::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        
        HTTP::Client *client = *it;
        if (client == NULL) {
            continue ;
        }

        client->checkTimeout();

        if (client->links == 0 && !client->processing()) {
            *it = NULL;
            addToDelClientQ(client);
        }
    }
}

void
Server::checkSessionsTimeout(void) {

    for (SessionsMap::iterator it = _sessions.begin(); it != _sessions.end();) {
        isActualSession((it++)->first);
    }
}

bool
Server::isActualSession(const std::string &s_id) {
    if (_sessions.find(s_id) != _sessions.end()) {
        std::time_t current = Time::now();
        if (current - _sessions[s_id] < settings.session_lifetime) {
            return true;
        }
        _sessions.erase(s_id);
    }
    return false;
}

void
Server::addSession(const std::string s_id) {

    pthread_mutex_lock(&_m_sessions);

    _sessions[s_id] = Time::now();

    pthread_mutex_unlock(&_m_sessions);
}

void
Server::addClient(HTTP::Client *client) {

    iter_cv it = std::find_if(_clients.begin(), _clients.end(), isClientFree);
    if (it == _clients.end()) {
        it = _clients.insert(_clients.end(), client);
    }
     
    *it = client;

    size_t id = std::distance(_clients.begin(), it);

    client->setId(id);
}

void
Server::link(int fd, HTTP::Client *client) {

    pthread_mutex_lock(&_m_link);

    _connector[fd] = client->getId();
    client->links++;
    
    addToNewFdsQ(fd);

    pthread_mutex_unlock(&_m_link);
}

void
Server::unlink(int fd) {

    // Log.debug() << "Server::unlink: fd = " << fd << Log.endl;

    pthread_mutex_lock(&_m_link);

    int id = _connector[fd];
    // Log.debug() << "Server::unlink: id = " << id << Log.endl;

    if (id < 0) {
        pthread_mutex_unlock(&_m_link);
        return ;
    }

    HTTP::Client *client = _clients[id];
    if (client == NULL) {
        pthread_mutex_unlock(&_m_link);
        return ;
    }
    // Log.debug() << "Server::unlink: client addr = " << client << Log.endl;

    if (fd >= 0) {
        close(fd);
    }

    _connector[fd] = -1;
    client->links--;

    addToDelFdsQ(fd);

    pthread_mutex_unlock(&_m_link);
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

    struct hostent *he = NULL;
    he = gethostbyaddr(&clientData.sin_addr, sizeof(clientData.sin_addr), AF_INET);
    if (he != NULL && he->h_name != NULL) {
        client->setDomainName(he->h_name);
    }

    client->setServerIO(_sockets[servid]);
    client->setClientTimeout(Time::now());
    client->getClientIO()->rdFd(fd);
    client->getClientIO()->wrFd(fd);
    client->getClientIO()->nonblock();
    client->getClientIO()->setAddr(inet_ntoa(clientData.sin_addr));
    client->getClientIO()->setPort(ntohs(clientData.sin_port));

    addClient(client);
    link(fd, client);

    Log.debug() << "Server::connect [" << fd << "] -> " << client->getHostname() << Log.endl;
}

// The functions below used to work with different queues

void Server::addToNewFdsQ(int fd) {

    if (fd < 0) {
        return ;
    }

    pthread_mutex_lock(&_m_new_pfds);

    Log.debug() << "Server::addToNewFdsQ [" << fd << "]" << Log.endl;
    _q_newPfds.push(fd);

    pthread_mutex_unlock(&_m_new_pfds);
}

void Server::addToNewClientQ(int fd, HTTP::Client *client) {

    pthread_mutex_lock(&_m_new_clnt);

    Log.debug() << "Server::addToNewClientQ [" << fd << "]" << Log.endl;
    _q_newClients.push(std::make_pair(fd, client));

    pthread_mutex_unlock(&_m_new_clnt);
}

void Server::addToDelFdsQ(int fd) {

    if (fd < 0) {
        return ;
    }

    pthread_mutex_lock(&_m_del_pfds);

    bool inserted = _q_delPfds.insert(fd).second;
    if (inserted) {
        Log.debug() << "Server::addToDelFdsQ [" << fd << "]" << Log.endl;
    }

    pthread_mutex_unlock(&_m_del_pfds);
}

void Server::addToDelClientQ(HTTP::Client *client) {

    pthread_mutex_lock(&_m_del_clnt);

    if (client != NULL) {
        bool inserted = _q_delClients.insert(client).second;
        if (inserted) {
            Log.debug() << "Server::addToDelClientQ -> " << client << Log.endl;
        }
    }

    pthread_mutex_unlock(&_m_del_clnt);
}

void Server::emptyNewFdsQ(void) {

    pthread_mutex_lock(&_m_new_pfds);

    while (!_q_newPfds.empty()) {

        int tmpfd = _q_newPfds.front();
        _q_newPfds.pop();

        struct pollfd tmp = { tmpfd, POLLIN | POLLOUT, 0 };

        iter_pfd it = std::find_if(_pollfds.begin(), _pollfds.end(), isFdFree);
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

    while (_q_delPfds.begin() != _q_delPfds.end()) {

        std::set<int>::iterator beg = _q_delPfds.begin();
        int tmpfd = *beg;
        _q_delPfds.erase(beg);

        if (tmpfd == -1) {
            continue;
        }
 
        struct pollfd tmp = { -1, 0, 0 };

        for (size_t i = 0; i < _pollfds.size(); ++i) {
            if (_pollfds[i].fd == tmpfd) {
                _pollfds[i] = tmp;
                break ;
            }
        }

        Log.debug() << "Server::emptyDelFdsQ [" << tmpfd << "]" << Log.endl;
    }

    pthread_mutex_unlock(&_m_del_pfds);
}

void Server::emptyNewClientQ(void) {
    pthread_mutex_lock(&_m_new_clnt);

    while (!_q_newClients.empty()) {
        std::pair<int, HTTP::Client *> tmp = _q_newClients.front();
        _q_newClients.pop();

        _clients[tmp.first] = tmp.second;
    
        Log.debug() << "Server::emptyNewClientQ [" << tmp.first << "]" << Log.endl;
    }

    pthread_mutex_unlock(&_m_new_clnt);
}

void Server::emptyDelClientQ(void) {

    pthread_mutex_lock(&_m_del_clnt);

    while (_q_delClients.begin() != _q_delClients.end()) {

        std::set<HTTP::Client *>::iterator beg = _q_delClients.begin();
        HTTP::Client *client = *beg;
        _q_delClients.erase(beg);

        rmClientFromRespQ(client);

        // int cio_r = client->getClientIO()->rdFd();
        // int gio_r = client->getGatewayIO()->rdFd();
        // int gio_w = client->getGatewayIO()->wrFd();

        _clients[client->getId()] = NULL;

        // close(cio_r);
        // close(gio_r);
        // if (gio_r != gio_w) {
        //     close(gio_w);
        // }

        Log.debug() << "Server::emptyDelClientQ -> " << client << Log.endl;

        delete client;
    }

    pthread_mutex_unlock(&_m_del_clnt);
}

void Server::rmClientFromRespQ(HTTP::Client *client) {

    pthread_mutex_lock(&_m_new_resp);
        
    std::list<HTTP::Response *>::iterator it;
    for (it = _q_newResponses.begin(); it != _q_newResponses.end(); ) {
        if (*it != NULL && (*it)->getClient() == client) {
            it = _q_newResponses.erase(it);
        } else {
            ++it;
        }
    }

    pthread_mutex_unlock(&_m_new_resp);
}

void Server::addToRespQ(HTTP::Response *res) {

    pthread_mutex_lock(&_m_new_resp);

    _q_newResponses.push_back(res);
    
    pthread_mutex_unlock(&_m_new_resp);
}

HTTP::Response *Server::rmFromRespQ(void) {

    HTTP::Response *res = NULL;

    pthread_mutex_lock(&_m_new_resp);

    if (_q_newResponses.begin() != _q_newResponses.end()) {
        res = _q_newResponses.front();
        _q_newResponses.pop_front();
    }

    pthread_mutex_unlock(&_m_new_resp);

    return res;
}
