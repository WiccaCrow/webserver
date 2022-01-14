#pragma once

#include <cstddef>
#include <cstring>  
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include <poll.h>   // poll настройка
#include <netdb.h>  // для struct addrinfo hints;
#include <arpa/inet.h>
#include <vector>


// #include "JSON.hpp"

#ifndef SOMAXCONN
# define SOMAXCONN 128
#endif 

#define TCP_SIZE 65536//максимальный размер пакета в TCP

class Response {};
class Request {};

class Client {
    private:
        int  _fd;           // переделано из HTTPreq
        char method[16];    // Метод
        char path[2048];    // URN
        char proto[16];     // Версия протокола
};

class Server {

    private:
        /* Variables */
        std::string _addr;   // Потом будет браться из конфига
        uint16_t    _port; 
        int32_t     _servfd;
        std::vector<struct pollfd> _pollfds;
void    recvServ(struct pollfd & pollCli);

    public:

        /* Constructs and destructs*/
        Server(const std::string &_addr, const uint16_t _port);
        ~Server();

        /* Operators */
        /* Set atributs */
        void    resetPollEvents(void);

        /* Get and show atributs */
        int     getServFd(void);

        /* other methods */
        void    startServ(void);
        void    acceptNewClient(void);
        void    disconnectClient(int i);
        void    recvServ(struct pollfd pollCli, int i);
        void    sendServ(struct pollfd pollCli, int i);
};
