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

class Response;

class Proxy {

public:
    typedef std::vector<std::string>    DomainsVec;

private:
    DomainsVec    _domains;
    URI           _pass;

    std::string  _host;
    std::string  _port;

public:
    Proxy(void);
    ~Proxy(void);

    int pass(Response *);
    void prepare(Response *);
    int setConnection(struct addrinfo *, Response *);

    int writeToSocket(int fd, std::string toWrite);

    std::string makeStartLine(Response *);

    DomainsVec          &getDomainsRef(void);
    const DomainsVec    &getDomainsRef(void) const;

    URI           &getPassRef(void);

};

}
