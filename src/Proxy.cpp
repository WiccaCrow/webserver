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

void Proxy::prepare(void) {
    IO *io = _res->getClient()->getTargetIO();

    std::string toWrite;
    toWrite.reserve(512);
    toWrite = makeStartLine() + CRLF;
    Headers<RequestHeader>::iterator it = _res->getRequest()->headers.begin();
    for (; it != _res->getRequest()->headers.end(); ++it) {
        if (it->first == CONNECTION || it->first == KEEP_ALIVE) {
            continue;
        }
        toWrite += it->second.key + ':' + it->second.value + CRLF;
    }
    toWrite += CRLF;
    _res->getRequest()->setHead(toWrite);

    writeToSocket(io->wrFd(), _res->getRequest()->getHead());
    writeToSocket(io->wrFd(), _res->getRequest()->getBody());
}

int Proxy::pass(Response *res) {
    _res = res;

    const std::string &host = res->getRequest()->getUriRef()._host;
    const std::string &port = res->getRequest()->getUriRef()._port_s;

    struct addrinfo *addrlst = NULL;
    if (getaddrinfo(host.c_str(), port.c_str(), NULL, &addrlst)) {
        Log.error() << "Proxy::getaddrinfo -> " << host << ":" << port << std::endl;
        return 0;
    }

    if (!setConnection(addrlst)) {
        freeaddrinfo(addrlst);
        return 0;
    }

    freeaddrinfo(addrlst);
    return 1;
}

int Proxy::setConnection(struct addrinfo *lst) {
    IO *sock = new IO();
    int fd = sock->create();

    if (fd < 0) {
        Log.error() << "Proxy:: Cannot create socket" << Log.endl;
        return 0;
    }

    for (; lst; lst = lst->ai_next) {
        if (lst->ai_socktype != SOCK_STREAM) {
            continue;
        }

        if (sock->connect(lst->ai_addr, lst->ai_addrlen)) {
            break;
        }
    }

    if (lst == NULL) {
        Log.error() << "Proxy:: Failed [" << fd << "] -> " << _host << ":" << _port << std::endl;
        return 0;
    }

    struct sockaddr_in *addr = (struct sockaddr_in *)lst->ai_addr;
    Log.info() << "Proxy:: Established [" << fd << "] -> " << inet_ntoa(addr->sin_addr) << std::endl;

    _res->getClient()->setTargetIO(sock);
    g_server->addToQueue((struct pollfd){fd, POLLIN, 0});
    g_server->addClient(fd, _res->getClient());

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
Proxy::makeStartLine(void) {
    Request *req = _res->getRequest();
    URI     &uri = req->getUriRef();

    if (uri._path.empty()) {
        if (req->getMethod() == "OPTIONS") {
            uri._path = "*";
        } else {
            uri._path = "/";
        }
    }
    req->headers[HOST].value = uri._host + ":" + uri._port_s;
    return req->getMethod() + " " + uri._path + " " + SERVER_PROTOCOL;
}

} // namespace HTTP
