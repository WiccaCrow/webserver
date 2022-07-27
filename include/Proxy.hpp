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
    typedef std::vector<std::string>    DomainsVec;

private:
    DomainsVec    _domains;
    URI           _pass;

public:
    Proxy(void);
    ~Proxy(void);

    int pass(Request *);
    int setConnection(struct addrinfo *, Request *);

    DomainsVec          &getDomainsRef(void);
    const DomainsVec    &getDomainsRef(void) const;

    URI           &getPassRef(void);

};

}
