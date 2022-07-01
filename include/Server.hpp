#pragma once

#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <vector>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <list>

#include "Utils.hpp"
#include "Client.hpp"
#include "Logger.hpp"
#include "Worker.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerBlock.hpp"

class Server {

public:
    typedef std::list<HTTP::ServerBlock> ServersList;
    typedef ServersList::iterator iter_sl;
    
    typedef std::map<std::size_t, ServersList> ServersMap;
    typedef ServersMap::iterator iter_sm;

    typedef std::vector<struct pollfd> PollFdVec;
    typedef PollFdVec::iterator iter_pfd;

    typedef std::vector<Socket *> SocketsVec;
    typedef SocketsVec::iterator iter_sv;

    typedef std::map<int, HTTP::Client *> ClientsMap;
    typedef ClientsMap::iterator iter_cm;

private:
    ServersMap    _servers;
    SocketsVec    _sockets;
    ClientsMap    _clients;
    PollFdVec     _pollfds;
    
    bool          _working;
    Worker        _workers[WORKERS];

    Pool<struct pollfd>  _waitingFds;

public:
    bool isDaemon;
    Pool<HTTP::Response *> responses;

    Server(void);
    Server(const Server &);
    ~Server(void);
    Server &operator=(const Server &);

    ServersList &operator[](std::size_t port);
    ServersMap &getServerBlocks(void);
    
    bool working(void);
    void start(void);
    void finish(void);

    void addClient(int fd, HTTP::Client * = NULL);
    
    void queuePollFd(int fd, int events);
    void addPollFd(void);
    void rmPollFd(int fd);

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
