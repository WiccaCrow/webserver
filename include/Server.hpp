#pragma once

#include <cstddef>
#include <cstring>  
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include <poll.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>

// #include "JSON.hpp"
#include "Request.hpp"

#ifndef SOMAXCONN
# define SOMAXCONN 128
#endif 

// максимальный размер пакета в TCP
#define PACKET_SIZE 65536 


class Response {};


class Client {
    private:
        int  _fd;           // переделано из HTTPreq
};

class Server {

    private:
        /* Variables */
        std::string                 _addr;
        uint16_t                    _port;
        // std::string                 _url; //from config
        int32_t                     _servfd;
        std::vector<struct pollfd>  _pollfds;
    	int                         _pollResult;

    public:

        /* Constructs and destructs*/
        Server(const std::string &_addr, const uint16_t _port);
        Server(const Server &obj);
        ~Server();

        /* Operators */
        /* Set atributs */
        void    resetPollEvents(void);

        /* Get and show atributs */
        int     getServFd(void);

        /* other methods */
        void    start(void);
        void    pollServ(void);
        void    acceptNewClient(void);
        void    disconnectClient(size_t id);
        void    recvServ(size_t id);
        void    sendServ(size_t id);
};
