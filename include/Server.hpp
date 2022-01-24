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
#include <vector>

#include "Client.hpp"
#include "ReadSock.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerBlock.hpp"
#include "Utils.hpp"

#ifndef SOMAXCONN
#define SOMAXCONN 128
#endif

// максимальный размер пакета в TCP
#define PACKET_SIZE 65536

class Server {
    private:
    // Variables
    size_t                     _nbServBlocks;
    std::vector<ServerBlock>   _ServBlocks;
    std::vector<struct pollfd> _pollfds;
    std::vector<Client>        _clients;
    int                        _pollResult;

    // Methods
    void fillServBlocksFds(void);

    public:
    Server();
    Server(const std::string &_addr, const uint16_t _port);
    Server(const Server &obj);
    ~Server();

    Server &operator=(const Server &obj);

    // Get and show atributs

    void addServerBlocks(ServerBlock &servBlock);
    void addServerBlocks(const std::string &ipaddr, const uint16_t port);

    void start(void);
    void pollServ(void);
    void acceptNewClient(size_t id);
    void handleAcceptError();
    void handlePollError();
    int  pollInHandler(size_t id);
    void pollHupHandler(size_t id);
    void pollOutHandler(size_t id);
    void pollErrHandler(size_t id);
};
