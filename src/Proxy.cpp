#include "Proxy.hpp"
#include "Server.hpp"

namespace HTTP {

Proxy::Proxy(void) {}
Proxy::~Proxy(void) {}

Proxy::Proxy(const Proxy &other) {
    *this = other;
}

Proxy &
Proxy::operator=(const Proxy &other) {
    if (this != &other) {
        // Rewrite
    }
    return *this;
}

int
Proxy::pass(void) {

    int res = 0;
    struct addrinfo *addrlst = NULL;
    if (getaddrinfo(_host.c_str(), _port.c_str(), NULL, &addrlst)) {
        Log.error() << "Proxy::getaddrinfo -> " << _host << ":" << _port << std::endl;
        res = 0;
    }
 
    if (!setConnection(addrlst)) {
        res = 0;
    }

    freeaddrinfo(addrlst);
    return res;
}

int
Proxy::setConnection(struct addrinfo *lst) {

    IO *sock = new IO();

    // IO *sock = _res->getClient()->getTargetIO();
    int fd = sock->create();

    if (fd < 0) {
        Log.error() << "Proxy:: Cannot create socket" << Log.endl;
        return 0;
    }

    for (; lst; lst = lst->ai_next) {
        if (lst->ai_socktype != SOCK_STREAM) {
            continue ;
        }

        if (sock->connect(lst->ai_addr, lst->ai_addrlen)) {
            break ;
        }
    }

    if (lst == NULL) {
        Log.error() << "Proxy:: Failed [" << fd << "] -> " << _host << ":" << _port << std::endl;
        return 0;
    }

    struct sockaddr_in *addr = (struct sockaddr_in *)lst->ai_addr;
    Log.info() << "Proxy:: Established [" << fd << "] -> " << inet_ntoa(addr->sin_addr) << std::endl;

    _res->getClient()->setTargetIO(sock);
    g_server->queuePollFd(fd, POLLIN | POLLOUT);
    g_server->addClient(fd, _res->getClient());

    return 1;
}

}
