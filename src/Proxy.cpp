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

void Proxy::prepare(Request *req) {
    // IO *io = res->getClient()->getGatewayIO();

    std::string toWrite;
    toWrite.reserve(512);
    toWrite = makeStartLine(req) + CRLF;
    Headers<RequestHeader>::iterator it = req->headers.begin();
    for (; it != req->headers.end(); ++it) {
        if (it->first == CONNECTION || it->first == KEEP_ALIVE) {
            continue;
        }
        toWrite += it->second.toString() + CRLF;
    }
    toWrite += CRLF;
    req->setHead(toWrite);
    // writeToSocket(io->wrFd(), res->getRequest()->getHead());
    // writeToSocket(io->wrFd(), res->getRequest()->getBody());
}

int Proxy::pass(Request *req) {

    const std::string &host = req->getUriRef()._host;
    const std::string &port = req->getUriRef()._port_s;

    struct addrinfo *addrlst = NULL;
    Log.error() << "Proxy::pass for " << host << ":" << port << Log.endl;
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

    g_server->link(sock->rdFd(), req->getClient());
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
Proxy::makeStartLine(Request *req) {

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
