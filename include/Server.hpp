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
#include "AClient.hpp"
#include "Client.hpp"
#include "Logger.hpp"
#include "Worker.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerBlock.hpp"
#include "Pool.hpp"

#ifndef SOMAXCONN
    # define SOMAXCONN 128
#endif

class Server {

public:
    typedef std::list<HTTP::ServerBlock> ServersList;
    typedef ServersList::iterator iter_sl;
    
    typedef std::map<size_t, ServersList> ServersMap;
    typedef ServersMap::iterator iter_sm;

    typedef std::vector<struct pollfd> PollFdVector;
    typedef PollFdVector::iterator iter_pfd;

    typedef std::vector<HTTP::AClient *> ClientVector;
    typedef PollFdVector::iterator iter_cv;

private:
    ServersMap    _serverBlocks;
    PollFdVector  _pollfds;
    ClientVector  _clients;
    size_t        _socketsCount;
    
    bool          _working;
    Worker        _workers[WORKERS];

public:
    Server(void);
    Server(const Server &other);
    ~Server(void);
    Server &operator=(const Server &other);

    void   addServerBlock(HTTP::ServerBlock &servBlock);
    ServersList &getServerBlocks(size_t port);
    
    bool isWorking(void);
    void start(void);
    void finish(void);
    
    // Thread-safe container
    Pool<HTTP::Request *> requests;
    Pool<HTTP::Response *> responses;
    size_t addSocket(struct pollfd, HTTP::AClient * = NULL);

private:
    void connectClient(size_t id);
    void disconnectClient(size_t id);
    
    int  poll(void);
    void process(void);

    void freeResponsePool(void);
    void createSockets(void);
    void startWorkers(void);
    void stopWorkers(void);

    int createListenSocket(const std::string &addr, size_t port);
    int addListenSocket(const std::string &addr, size_t port);
};
