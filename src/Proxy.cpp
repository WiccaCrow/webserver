#include "Proxy.hpp"

#include "Response.hpp"
#include "Server.hpp"

namespace HTTP {

Proxy::Proxy(const std::string &host, const std::string &port)
    : _host(host)
    , _port(port) {}

Proxy::~Proxy(void) {}

int Proxy::pass(Request *req) {

    if (_port.empty()) {
        _port = "80";
    }

    struct addrinfo *addrlst = NULL;
    Log.debug() << "Try to proxy to " << _host << ":" << _port << Log.endl;
    if (getaddrinfo(_host.c_str(), _port.c_str(), NULL, &addrlst)) {
        Log.error() << "Proxy::getaddrinfo -> " << _host << ":" << _port << Log.endl;
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
