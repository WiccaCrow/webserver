#pragma once

#ifndef SERVER_SOFTWARE
    # define SERVER_SOFTWARE "Webserv/1.0.0"
#endif

#ifndef GATEWAY_INTERFACE
    # define GATEWAY_INTERFACE "CGI/1.1"
#endif

class Server;

extern Server *g_server;
