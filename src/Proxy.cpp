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

void Proxy::prepare(Response *res) {
    // IO *io = res->getClient()->getGatewayIO();

    std::string toWrite;
    toWrite.reserve(512);
    toWrite = makeStartLine(res) + CRLF;
    Headers<RequestHeader>::iterator it = res->getRequest()->headers.begin();
    for (; it != res->getRequest()->headers.end(); ++it) {
        if (it->first == CONNECTION || it->first == KEEP_ALIVE) {
            continue;
        }
        toWrite += it->second.toString() + CRLF;
    }
    toWrite += CRLF;
    res->getRequest()->setHead(toWrite);
    // writeToSocket(io->wrFd(), res->getRequest()->getHead());
    // writeToSocket(io->wrFd(), res->getRequest()->getBody());
}

int Proxy::pass(Response *res) {

    const std::string &host = res->getRequest()->getUriRef()._host;
    const std::string &port = res->getRequest()->getUriRef()._port_s;

    struct addrinfo *addrlst = NULL;
    Log.error() << "Proxy::pass for " << host << ":" << port << Log.endl;
    if (getaddrinfo(host.c_str(), port.c_str(), NULL, &addrlst)) {
        Log.error() << "Proxy::getaddrinfo -> " << host << ":" << port << Log.endl;
        return 0;
    }

    if (!setConnection(addrlst, res)) {
        freeaddrinfo(addrlst);
        return 0;
    }

    freeaddrinfo(addrlst);
    return 1;
}

int Proxy::setConnection(struct addrinfo *lst, Response *res) {

    IO *sock = res->getClient()->getGatewayIO();
    
    int fd = sock->create();
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
        Log.debug() << "Proxy:: Try connect [" << fd << "] -> " << inet_ntoa(addr->sin_addr) << Log.endl;
    }

    if (!connected && lst == NULL) {
        Log.error() << "Proxy:: Failed [" << fd << "] -> " << _host << ":" << _port << Log.endl;
        return 0;
    }

    if (sock->nonblock() < 0) {
        Log.error() << "Proxy:: nonblock failed" << Log.endl;
        return 0;
    }

    Log.info() << "Proxy:: Established [" << fd << "]" << Log.endl;

    g_server->addToNewClientQ(fd, res->getClient());
    g_server->addToNewFdsQ(fd);

    return 1;
}

int Proxy::writeToSocket(int fd, std::string toWrite) {

    if (write(fd, toWrite.c_str(), toWrite.length()) == -1) {
        Log.syserr() << "Proxy::write: " << Log.endl;
        return 0;
    }
    return 1;
}

std::string
Proxy::makeStartLine(Response *res) {
    Request *req = res->getRequest();
    URI     &uri = req->getUriRef();

    if (uri._path.empty()) {
        if (req->getMethod() == "OPTIONS") {
            uri._path = "*";
        } else {
            uri._path = "/";
        }
    }
    if (!uri._host.empty()) {
        req->headers[HOST].value = uri._host + ":" + uri._port_s;
    }
    return req->getMethod() + " " + uri._path + " " + SERVER_PROTOCOL;
}

} // namespace HTTP
