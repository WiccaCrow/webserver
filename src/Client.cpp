#include "Client.hpp"

#include "Server.hpp"

namespace HTTP {

Client::Client(void)
    : _clientIO(NULL), _serverIO(NULL), _gatewayIO(NULL), _shouldBeClosed(false), _shouldBeRemoved(false), _isTunnel(false), _nbRequests(0), _maxRequests(MAX_REQUESTS), _clientTimeout(0), _gatewayTimeout(0) {
    _clientIO = new IO();
    if (_clientIO == NULL) {
        shouldBeRemoved(true);
        Log.syserr() << "Client:: Cannot allocate memory for client socket" << Log.endl;
    }

    _gatewayIO = new IO();
    if (_gatewayIO == NULL) {
        shouldBeRemoved(true);
        Log.syserr() << "Client:: Cannot allocate memory for gateway socket" << Log.endl;
    }
}

Client::~Client(void) {
    typedef std::list<Request *>  PoolReq;
    typedef std::list<Response *> PoolRes;

    for (PoolReq::iterator it = _requests.begin(); it != _requests.end(); ++it) {
        if (*it != NULL) {
            delete *it;
        }
    }

    for (PoolRes::iterator it = _responses.begin(); it != _responses.end(); ++it) {
        if (*it != NULL) {
            delete *it;
        }
    }

    if (_gatewayIO) {
        delete _gatewayIO;
    }

    if (_clientIO) {
        delete _clientIO;
    }
}

void Client::shouldBeClosed(bool flag) {
    _shouldBeClosed = flag;
}

bool Client::shouldBeClosed(void) const {
    return _shouldBeClosed;
}

void Client::shouldBeRemoved(bool flag) {
    _shouldBeRemoved = flag;
}

bool Client::shouldBeRemoved(void) const {
    return _shouldBeRemoved;
}

bool Client::isTunnel(void) const {
    return _isTunnel;
}

void Client::isTunnel(bool value) {
    _isTunnel = value;
}

time_t
Client::getClientTimeout(void) const {
    return _clientTimeout;
}

time_t
Client::getGatewayTimeout(void) const {
    return _gatewayTimeout;
}

void Client::setClientTimeout(time_t time) {
    _clientTimeout = time;
}

void Client::setGatewayTimeout(time_t time) {
    _gatewayTimeout = time;
}

IO *Client::getClientIO(void) {
    return _clientIO;
}

IO *Client::getServerIO(void) {
    return _serverIO;
}

IO *Client::getGatewayIO(void) {
    return _gatewayIO;
}

void Client::setClientIO(IO *sock) {
    _clientIO = sock;
}

void Client::setServerIO(IO *sock) {
    _serverIO = sock;
}

void Client::setGatewayIO(IO *sock) {
    _gatewayIO = sock;
}

const std::string
Client::getHostname(void) {
    const std::size_t  port = getClientIO()->getPort();
    const std::string &addr = getClientIO()->getAddr();

    return (port != 0 ? addr + ":" + sztos(port) : addr);
}

void Client::addRequest(void) {
    Request *req = new Request(this);
    if (req == NULL) {
        Log.syserr() << "Cannot allocate memory for Request" << Log.endl;
        shouldBeRemoved(true);
        return;
    }
    _requests.push_back(req);

    Log.debug() << "------------------------------------------" << Log.endl;
}

void Client::addResponse(void) {
    Request  *req = _requests.back();
    Response *res = new Response(req);

    if (req == NULL) {
        Log.syserr() << "Cannot allocate memory for Response" << Log.endl;
        shouldBeRemoved(true);
        return;
    }
    _responses.push_back(res);

    Log.debug() << "Client::addResponse " << res->getRequest()->getUriRef()._path << Log.endl;
}

void Client::removeRequest(void) {
    if (_requests.size() > 0) {
        Request *req = _requests.front();
        _requests.pop_front();
        delete req;
    }
}

void Client::removeResponse(void) {
    if (_responses.size() > 0) {
        Response *res = _responses.front();
        _responses.pop_front();
        delete res;
    }
}

void Client::pollin(int fd) {
    if (fd == getClientIO()->rdFd()) {
        if (_requests.size() == _responses.size()) {
            addRequest();
        }

        if (_requests.size() > _responses.size()) {
            setClientTimeout(Time::now());
            receive(_requests.back());

            if (_requests.back()->formed()) {
                if (_nbRequests++ >= _maxRequests) {
                    shouldBeClosed(true);
                }
                addResponse();
                g_server->addToRespQ(_responses.back());
            }
        }
    } else if (fd == getGatewayIO()->rdFd()) {
        if (_responses.size() > 0) {
            receive(_responses.front());
        }
    }
}

void Client::pollout(int fd) {

    if (fd == getClientIO()->wrFd()) {

        if (_responses.empty()) {
            return ;
        }

        HTTP::Response *res = _responses.front();
        if (res->formed() && !res->sent()) {
            reply(res);
        }

        if (res->sent()) {
            removeRequest();
            removeResponse();

            getClientIO()->clear();

            if (!isTunnel() && getGatewayIO()) {
                g_server->addToDelFdsQ(getGatewayIO()->rdFd());
            }

            if (shouldBeClosed()) {
                shouldBeRemoved(true);
            }
        }

    } else if (fd == getGatewayIO()->wrFd()) {

        if (_requests.empty()) {
            return ;
        }

        HTTP::Request *req = _requests.front();
        if (req->formed() && !req->sent()) {
            reply(req);
        }
    }
}

void Client::pollhup(int fd) {
    Log.syserr() << "Client::pollhup [" << fd << "]" << Log.endl;
    if (fd == getClientIO()->rdFd()) {
        g_server->addToDelFdsQ(getClientIO()->rdFd());
        g_server->addToDelFdsQ(getGatewayIO()->rdFd());
        g_server->addToDelFdsQ(getGatewayIO()->wrFd());

    } else {
        g_server->addToDelFdsQ(getGatewayIO()->rdFd());
        g_server->addToDelFdsQ(getGatewayIO()->wrFd());
    }
}

void Client::pollerr(int fd) {
    Log.syserr() << "Client::pollerr [" << fd << "]" << Log.endl;

    if (fd == getClientIO()->rdFd()) {
        g_server->addToDelFdsQ(getClientIO()->rdFd());
        g_server->addToDelFdsQ(getGatewayIO()->rdFd());
        g_server->addToDelFdsQ(getGatewayIO()->wrFd());
    } else {
        g_server->addToDelFdsQ(getGatewayIO()->rdFd());
        g_server->addToDelFdsQ(getGatewayIO()->wrFd());
    }
}

void Client::reply(Response *res) {
    signal(SIGPIPE, SIG_IGN);

    if (!res->headSent()) {
        if (!getClientIO()->getDataPos()) {
            // Log.debug() << res->getHead() << Log.endl;
            getClientIO()->setData(res->getHead().c_str());
            getClientIO()->setDataSize(res->getHead().length());
        }

    } else if (!res->bodySent()) {
        if (!getClientIO()->getDataPos()) {
            // Log.debug() << res->getBody() << Log.endl;
            getClientIO()->setData(res->getBody().c_str());
            getClientIO()->setDataSize(res->getBody().length());
        }
    }

    int bytes = getClientIO()->write();
    
    if (bytes < 0) {
        return ;
    }
    
    if (bytes == 0) {
        // if Proxy-tunnel close Client and Gateway sockets
        return ;
    }

    if (static_cast<std::size_t>(bytes) >= getClientIO()->getDataSize()) {
        if (!res->headSent()) {
            res->headSent(true);

        } else if (!res->bodySent()) {
            res->bodySent(true);
        }
    }
}

void Client::reply(Request *req) {

    signal(SIGPIPE, SIG_IGN);

    if (!req->headSent()) {
        if (!getGatewayIO()->getDataPos()) {
            getGatewayIO()->setData(req->getHead().c_str());
            getGatewayIO()->setDataSize(req->getHead().length());
        }

    } else if (!req->bodySent()) {
        if (!getGatewayIO()->getDataPos()) {
            getGatewayIO()->setData(req->getBody().c_str());
            getGatewayIO()->setDataSize(req->getBody().length());
        }
    }

    int bytes = getGatewayIO()->write();
    
    if (bytes < 0) {
        return ;
    }
    
    if (bytes == 0) {
        // set response status as BAD_GATEWAY
        // if CGI close pipe (standard action)
        // if Proxy-req close only GATEWAY socket
        // if Proxy-tunnel close Client and Gateway sockets
        return ;
    }

    if (static_cast<std::size_t>(bytes) >= getGatewayIO()->getDataSize()) {
        if (!req->headSent()) {
            req->headSent(true);
        } else if (!req->bodySent()) {
            req->bodySent(true);
        }
    } 

    if (req->sent()) {
        setGatewayTimeout(Time::now());
    }
}

void Client::receive(Request *req) {

    int bytes = getClientIO()->read();

    if (bytes < 0) {
        Log.debug() << "Client::receive [" << getClientIO()->rdFd() << "] req not ready" << Log.endl;
        return ;

    } else if (bytes == 0) {
        Log.debug() << "Client::receive [" << getClientIO()->rdFd() << "] peer closed connection" << Log.endl;
        shouldBeRemoved(true);
        return ;
    }

    while (!req->formed()) {
        std::string line;

        if (!getClientIO()->getline(line, req->getBodySize())) {
            return ;
        }

        req->parseLine(line);
    }
}

void Client::receive(Response *res) {

    int bytes = getGatewayIO()->read();

    if (bytes < 0) {
        Log.debug() << "Client::receive [" << getGatewayIO()->rdFd() << "] resp not ready" << Log.endl;
        return ;

    } else if (bytes == 0) {
        Log.debug() << "Client::receive [" << getGatewayIO()->rdFd() << "] resp done" << Log.endl;

        if (res->isCGI()) {
            res->checkCGIFail();
        }
        
        g_server->addToDelFdsQ(getGatewayIO()->rdFd());
        getGatewayIO()->closeRdFd();
    }
    
    setGatewayTimeout(0);
    while (!res->formed()) {
        std::string line;

        if (res->getStatus() >= BAD_REQUEST) {
            res->assembleError();
        }

        if (!getGatewayIO()->getline(line, res->getBodySize())) {
            return ;
        }

        res->parseLine(line);
    }
}

ServerBlock *
Client::matchServerBlock(const std::string &host) {

    bool searchByName = !isValidIpv4(host) ? true : false;
    
    Server::ServersList &blocks = g_server->operator[](getServerIO()->getPort());
    
    Server::iter_sl found = blocks.end();
    for (Server::iter_sl block = blocks.begin(); block != blocks.end(); ++block) {
        if (block->hasAddr(getServerIO()->getAddr())) {
            if (found == blocks.end()) {
                found = block;
                if (!searchByName) {
                    break ;
                }
            }
            if (searchByName && block->hasName(host)) {
                found = block;
                break ;
            }
        }
    }
    Log.debug() << "Client::matchServerBlock: " << found->getBlockName() << " for " << host << ":" << getServerIO()->getPort() << Log.endl;
    return &(*found);
}

} // namespace HTTP
