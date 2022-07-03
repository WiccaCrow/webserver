#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/resource.h>
#include <unistd.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <queue>
#include <vector>

#include "Client.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerBlock.hpp"
#include "Utils.hpp"
#include "Worker.hpp"

class Server {
    public:
    typedef std::list<HTTP::ServerBlock> ServersList;
    typedef ServersList::iterator        iter_sl;

    typedef std::map<std::size_t, ServersList> ServersMap;
    typedef ServersMap::iterator               iter_sm;

    typedef std::vector<struct pollfd> PollFdVec;
    typedef PollFdVec::iterator        iter_pfd;

    typedef std::vector<IO *>    SocketsVec;
    typedef SocketsVec::iterator iter_sv;

    typedef std::map<int, HTTP::Client *> ClientsMap;
    typedef ClientsMap::iterator          iter_cm;

    private:
    ServersMap _servers;
    SocketsVec _sockets;
    ClientsMap _clients;
    PollFdVec  _pollfds;

    bool   _working;
    Worker _workers[WORKERS];

    pthread_mutex_t _fds_lock;
    pthread_mutex_t _res_lock;

    std::queue<struct pollfd>    _pendingFds;
    std::queue<HTTP::Response *> _pendingResps;

    public:
    bool isDaemon;

    Server(void);
    Server(const Server &);
    ~Server(void);
    Server &operator=(const Server &);

    ServersList &operator[](std::size_t port);
    ServersMap  &getServerBlocks(void);

    bool working(void);
    void start(void);
    void finish(void);

    void addClient(int fd, HTTP::Client * = NULL);

    void rmPollFd(int fd);
    void emptyFdsQueue(void);
    void addToQueue(struct pollfd);
    void addToQueue(HTTP::Response *);

    HTTP::Response *rmFromQueue(void);

    private:
    void connect(std::size_t servid, int servfd);
    void disconnect(int fd);

    void daemon(void);
    int  poll(void);
    void process(void);
    void checkTimeout(void);

    void createSockets(void);
    void startWorkers(void);
    void stopWorkers(void);

    int addListenSocket(const std::string &addr, std::size_t port);
};
