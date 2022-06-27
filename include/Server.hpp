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
#include "Request.hpp"
#include "Response.hpp"
#include "ServerBlock.hpp"

#ifndef SOMAXCONN
    # define SOMAXCONN 128
#endif

struct WorkerInfo {
    int id;
};

class Server {

public:
    typedef std::list<HTTP::ServerBlock> ServersList;
    typedef ServersList::iterator iter_sl;
    
    typedef std::map<size_t, ServersList> ServersMap;
    typedef ServersMap::iterator iter_sm;

    typedef std::vector<struct pollfd> PollFdVector;
    typedef PollFdVector::iterator iter_pfd;

    typedef std::vector<HTTP::Client *> ClientVector;
    typedef PollFdVector::iterator iter_cv;

private:
    ServersMap    _serverBlocks;
    PollFdVector  _pollfds;
    ClientVector  _clients;
    size_t        _socketsCount;
    
    bool          _working;
    pthread_t     _threads[WORKERS];
    WorkerInfo    _workerInfos[WORKERS];

public:
    Server(void);
    Server(const std::string &_addr, const uint16_t _port);
    Server(const Server &obj);
    ~Server(void);

    Server &operator=(const Server &obj);

    void   addServerBlock(HTTP::ServerBlock &servBlock);
    std::list<HTTP::ServerBlock> &getServerBlocks(size_t port);
    
    bool isWorking(void);

    void freeResponsePool(void);
    void createSockets(void);
    void createWorkers(void);
    void destroyWorkers(void);
    
    int  poll(void);
    void start(void);
    void finish(void);
    void connectClient(size_t id);
    void disconnectClient(size_t id);
    void pollInHandler(size_t id);
    void pollHupHandler(size_t id);
    void pollOutHandler(size_t id);
    void pollErrHandler(size_t id);


    int createListenSocket(const std::string &addr, size_t port);
    int addListenSocket(const std::string &addr, size_t port);
    size_t addSocket(struct pollfd, HTTP::Client * = NULL);
};
