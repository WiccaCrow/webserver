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

private:
    typedef std::map<size_t, std::list<HTTP::ServerBlock> >::iterator iter;

    std::map<size_t, std::list<HTTP::ServerBlock> > _serverBlocks;
    std::vector<struct pollfd>     _pollfds;
    std::vector<HTTP::Client *>    _clients;
    size_t                          _socketsCount;

    void fillServBlocksFds(void);
    int createListenSocket(const std::string &addr, size_t port);
    void addListenSocket(const std::string &addr, size_t port);

    pthread_t _threads[WORKERS];

public:
    WorkerInfo workerInfos[WORKERS];

    Server();
    Server(const std::string &_addr, const uint16_t _port);
    Server(const Server &obj);
    ~Server();

    Server &operator=(const Server &obj);

    void   addServerBlock(HTTP::ServerBlock &servBlock);
    std::list<HTTP::ServerBlock> &getServerBlocks(size_t port);
    
    void freeResponsePool(void);
    void createWorkers(void);
    void destroyWorkers(void);
    
    int  poll(void);
    void start(void);
    void connectClient(size_t id);
    void disconnectClient(size_t id);
    void handlePollError();
    void pollInHandler(size_t id);
    void pollHupHandler(size_t id);
    void pollOutHandler(size_t id);
    void pollErrHandler(size_t id);

    size_t addClient(int fd, HTTP::Client *);

};
