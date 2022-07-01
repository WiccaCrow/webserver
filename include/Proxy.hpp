#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "Globals.hpp"
#include "Logger.hpp"
#include "URI.hpp"
#include "Status.hpp"
#include "Response.hpp"

namespace HTTP {

class Proxy {

private:
    Response    *_res;

    std::string  _host;
    std::string  _port;

public:
    Proxy(void);
    ~Proxy(void);

    Proxy(const Proxy &);
    Proxy &operator=(const Proxy &);

    int pass(void);
    int setConnection(struct addrinfo *);
};

}
