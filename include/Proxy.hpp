#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "Globals.hpp"
#include "Logger.hpp"
#include "URI.hpp"
#include "Status.hpp"
#include "Header.hpp"

namespace HTTP {

class Request;

class Proxy {

public:

private:
    std::string _host;
    std::string _port;

public:
    Proxy(const std::string &host, const std::string &port);
    ~Proxy(void);

    int pass(Request *);
    int setConnection(struct addrinfo *, Request *);
};

}
