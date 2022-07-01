#include "Proxy.hpp"
#include "Server.hpp"

namespace HTTP {

Proxy::Proxy(void)
            : _uri(NULL)
            , _on(false)
            , _fdOut(-1)
            , _idOtherSide(0) {}

Proxy::Proxy(URI *uri)
            : _uri(uri)
            , _host(_uri->_scheme)
            , _port(_uri->_port_s)
            , _on(false)
            , _fdOut(-1)
            , _idOtherSide(0) {}

Proxy::~Proxy(void) {}

Proxy::Proxy(const Proxy &other) {
    *this = other;
}

Proxy &
Proxy::operator=(const Proxy &other) {
    if (this != &other) {
        _uri         = other._uri;
        _on          = other._on;
        _status      = other._status;
        _fdOut       = other._fdOut;
        _idOtherSide = other._idOtherSide;
        if (_uri) {
            _host        = _uri->_scheme;
            _port        = _uri->_port_s;
        }
    }
    return *this;
}

void
Proxy::clear(void) {
    _uri = NULL;
    _on  = false;
}

void
Proxy::setUri(URI *uri) {
    _uri  = uri;
    _host = _uri->_host;
    _port = _uri->_port_s;
}

std::size_t
Proxy::run(void) {
    // fill struct addrinfo
    addrinfo *ipListAddrinfo = NULL;
    if (getaddrinfo(_host.c_str(), _port.c_str(), NULL, &ipListAddrinfo)) {
        setStatus(INTERNAL_SERVER_ERROR);
        Log.error() << "Proxy::getaddrinfo for " << _host << ":" << _port << std::endl;
        return 0;
    }
 
    addrinfo *p = setConnection(ipListAddrinfo);
    // add fd for poll and client
    if (p && p->ai_family == AF_INET) {
        struct sockaddr_in *addr = (struct sockaddr_in *) p->ai_addr;
        _idOtherSide = g_server->addPollfd(_fdOut, POLLIN | POLLOUT);
        g_server->addClient(_idOtherSide, _res->getClient());
        on(true);
        setStatus(OK);
    }
    freeaddrinfo(ipListAddrinfo);
    
    return _idOtherSide;
}

addrinfo *
Proxy::setConnection(addrinfo *p) {
    for (; p; p = p->ai_next) {
        if (p->ai_socktype != SOCK_STREAM) {
            continue ;
        }
        _fdOut = g_server->connect(p->ai_addr, p->ai_addrlen);
        if (_fdOut == -1) {
            Log.debug() << "Proxy::connection failed" << std::endl;
            continue;
        }
        break ;
    }

    if (!p) {
        setStatus(BAD_GATEWAY);
        Log.debug() << "Proxy: for this " << _host << ":" << _port 
                    << " connection not possible: " << std::endl;
    }
    
    char ip[INET_ADDRSTRLEN];
    Log.info() << "Proxy: Connection Established (fd[" << _fdOut <<"]), ip: " << inet_ntop(AF_INET, &((struct sockaddr_in *) p->ai_addr)->sin_addr, ip, INET_ADDRSTRLEN)<< std::endl;

    return p;
}

void
Proxy::setStatus(StatusCode code) {
    _status = code;
}

StatusCode
Proxy::getStatus(void) {
    return _status;
}

void
Proxy::setFdOut(int fd) {
    _fdOut = fd;
}

int
Proxy::getFdOut(void) {
    return _fdOut;
}

bool
Proxy::on(void) {
    return _on;
}

void
Proxy::on(bool onOff) {
    _on = onOff;
}

void   
Proxy::idOtherSide(std::size_t id) {
    _idOtherSide = id;
}

std::size_t 
Proxy::idOtherSide(void) {
    return _idOtherSide;
}

}
