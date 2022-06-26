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

#include "PoolController.hpp"

#ifndef SOMAXCONN
    # define SOMAXCONN 128
#endif

class Server {

private:
    typedef std::map<size_t, std::list<HTTP::ServerBlock> >::iterator iter;

    std::map<size_t, std::list<HTTP::ServerBlock> > _serverBlocks;
    std::vector<struct pollfd>     _pollfds;
    std::map<size_t, HTTP::Client> _clients;
    int                            _pollResult;
    size_t              _socketsCount;
    void fillServBlocksFds(void);

    int addListenSocket(const std::string &addr, size_t port);

    pthread_t _threads[WORKERS];

public:
    PoolController poolCtl;
    PoolController wPoolCtls[WORKERS];

    Server();
    Server(const std::string &_addr, const uint16_t _port);
    Server(const Server &obj);
    ~Server();

    Server &operator=(const Server &obj);

    void   addServerBlock(HTTP::ServerBlock &servBlock);
    std::list<HTTP::ServerBlock> &getServerBlocks(size_t port);
    

    void freePool(void);
    void createWorkers(void);
    void destroyWorkers(void);
    
    void    start(void);
    void    pollServ(void);
    void    connectClient(size_t id);
    size_t  proxySetFdAndClient(int fd, struct sockaddr_in *cli4);
    size_t  addClient(size_t id, int fd, struct sockaddr_in *cli4, struct sockaddr_in *servData);
    size_t  addSockToPollfd(int fd);
    void    disconnectClient(size_t id);
    void    handlePollError();
    void    pollInHandler(size_t id);
    void    pollHupHandler(size_t id);
    void    pollOutHandler(size_t id);
    void    pollErrHandler(size_t id);

    HTTP::Client *getClient(size_t id);

};
