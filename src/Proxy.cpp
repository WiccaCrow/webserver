#include "Proxy.hpp"

#include "Response.hpp"
#include "Server.hpp"

namespace HTTP {

Proxy::Proxy(void) {}
Proxy::~Proxy(void) {}

Proxy::DomainsVec &
Proxy::getDomainsRef(void) {
    return _domains;
}

const Proxy::DomainsVec &
Proxy::getDomainsRef(void) const {
    return _domains;
}

URI &Proxy::getPassRef(void) {
    return _pass;
}

int Proxy::pass(Request *req) {

    std::string host;
    std::string port;

    if (!_pass._host.empty() && !_pass._port_s.empty()) {
        host = _pass._host;
        port = _pass._port_s;
    } else {
        host = req->getUriRef()._host;
        port = req->getUriRef()._port_s;
    }

    if (port.empty()) {
        port = "80";
    }

    struct addrinfo *addrlst = NULL;
    Log.debug() << "Try to proxy to " << host << ":" << port << Log.endl;
    if (getaddrinfo(host.c_str(), port.c_str(), NULL, &addrlst)) {
        Log.error() << "Proxy::getaddrinfo -> " << host << ":" << port << Log.endl;
        return 0;
    }

    if (!setConnection(addrlst, req)) {
        freeaddrinfo(addrlst);
        return 0;
    }

    freeaddrinfo(addrlst);
    return 1;
}

int Proxy::setConnection(struct addrinfo *lst, Request *req) {

    IO *sock = req->getClient()->getGatewayIO();
    
    int fd = sock->socket();
    if (fd < 0) {
        Log.error() << "Proxy:: Cannot create socket" << Log.endl;
        return 0;
    }

    int connected = 0;
    for (; lst && !connected; lst = lst->ai_next) {
        if (lst->ai_socktype != SOCK_STREAM) {
            continue;
        }

        connected = sock->connect(lst->ai_addr, lst->ai_addrlen);

        struct sockaddr_in *addr = (struct sockaddr_in *)lst->ai_addr;
        Log.debug() << "Proxy:: [" << fd << "] Try connect to " << inet_ntoa(addr->sin_addr) << ":" << ntohs(addr->sin_port) << Log.endl;
    }

    if (connected == -1) {
        Log.error() << "Proxy:: [" << fd << "] Connection failed" << Log.endl;
        return 0;
    }

    if (sock->nonblock() < 0) {
        Log.error() << "Proxy:: nonblock failed" << Log.endl;
        return 0;
    }

    struct sockaddr_in ownAddr;
    socklen_t ownAddrSize = sizeof(ownAddr);
    getsockname(sock->rdFd(), (struct sockaddr *)&ownAddr, &ownAddrSize);
    Log.debug() << "Proxy:: [" << fd << "] Established from " << inet_ntoa(ownAddr.sin_addr) << ":" << ntohs(ownAddr.sin_port) << Log.endl;

    g_server->link(sock->rdFd(), req->getClient());
    return 1;
}

} // namespace HTTP
