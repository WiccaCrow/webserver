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

#include "Utils.hpp"
#include "Client.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "ReadSock.hpp"
#include "Response.hpp"
#include "ServerBlock.hpp"

#ifndef SOMAXCONN
    # define SOMAXCONN 128
#endif

class Server {

private:
    std::vector<HTTP::ServerBlock> _servBlocks;
    std::vector<struct pollfd>     _pollfds;
    std::map<size_t, HTTP::Client> _clients;
    int                            _pollResult;

    void fillServBlocksFds(void);

    int createListenSocket(const std::string addr, const int port);

public:
    Server();
    Server(const std::string &_addr, const uint16_t _port);
    Server(const Server &obj);
    ~Server();

    Server &operator=(const Server &obj);

    void   addServerBlock(HTTP::ServerBlock &servBlock);

    void start(void);
    void pollServ(void);
    void connectClient(size_t id);
    void disconnectClient(size_t id);
    void handleAcceptError();
    void handlePollError();
    int  pollInHandler(size_t id);
    void pollHupHandler(size_t id);
    void pollOutHandler(size_t id);
    void pollErrHandler(size_t id);

    HTTP::ServerBlock *matchServerBlock(int port, const std::string &ipaddr, const std::string &host);
};
