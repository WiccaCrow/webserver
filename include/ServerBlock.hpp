#pragma once

#include <cstring>
#include <cstdint>
#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>


class ServerBlock {

    private:
        /* Variables */
        std::string                 _addr;
        uint16_t                    _port;
        // std::string                 _url; //from config
        int32_t                     _servfd;

		 /* Methods */
		void	createListenSock(void);
		void	reuseAddr(void);
		void	bindAddr(void);
		void	listenSock(void);

    public:

        /* Constructs and destructs*/
		ServerBlock();
        ServerBlock(const std::string &ipaddr, const uint16_t port);
        ServerBlock(const ServerBlock &obj);
        ~ServerBlock();

        /* Operators */
		ServerBlock	&operator=(const ServerBlock &obj);
        /* Set atributs */
        // void    resetPollEvents(void);

        /* Get and show atributs */
        int     getServFd(void);

        /* other methods */
        void    start();
};
