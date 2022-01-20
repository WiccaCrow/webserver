#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

// #include "JSON.hpp"
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

    ReadSock _reader;

    // Methods
    void assignPollFds(void);
    void fillServBlocksFds(void);

    public:
    Server();
    Server(const std::string &_addr, const uint16_t _port);
    Server(const Server &obj);
    ~Server();

    Server &operator=(const Server &obj);

    void resetPollEvents(void);

    // Get and show atributs

    void addServerBlocks(ServerBlock &servBlock);
    void addServerBlocks(const std::string &ipaddr, const uint16_t port);

    void start(void);
    void pollServ(void);
    void acceptNewClient(size_t id);
    void disconnectClient(size_t id);
    void recvServ(size_t id);
    void sendServ(size_t id);
};
