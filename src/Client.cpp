#include "Client.hpp"

#include "Server.hpp"

namespace HTTP {

Client::Client(void)
    : _clientIO(NULL), 
    _serverIO(NULL),
    _gatewayIO(NULL),
    _processing(false),
    _shouldBeClosed(false),
    _shouldBeRemoved(false),
    _isTunnel(false),
    _nbRequests(0),
    _maxRequests(g_server->settings.max_requests),
    _clientTimeout(0),
    _gatewayTimeout(0),
    _id(-1),
    links(0) 
{
    _clientIO = new IO();
    if (_clientIO == NULL) {
        // shouldBeRemoved(true);
        Log.syserr() << "Client:: Cannot allocate memory for client socket" << Log.endl;
    }

    _gatewayIO = new IO();
    if (_gatewayIO == NULL) {
        // shouldBeRemoved(true);
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

void Client::processing(bool flag) {
    _processing = flag;
}

bool Client::processing(void) const {
    return _processing;
}

void Client::shouldBeClosed(bool flag) {
    _shouldBeClosed = flag;
}

bool Client::shouldBeClosed(void) const {
    return _shouldBeClosed;
}

// Remove
// void Client::shouldBeRemoved(bool flag) {
//     _shouldBeRemoved = flag;
// }

// bool Client::shouldBeRemoved(void) const {
//     return _shouldBeRemoved;
// }

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

const std::string &
Client::getDomainName(void) const {
    return _domain;
}

void
Client::setDomainName(const std::string &name) {
    _domain = name;
}

const std::string
Client::getHostname(void) {
    const std::size_t  port = getClientIO()->getPort();
    const std::string &addr = getClientIO()->getAddr();

    return (port != 0 ? addr + ":" + sztos(port) : addr);
}

int
Client::getId(void) const {
    return _id;
}

void
Client::setId(int id) {
    _id = id;
}

void Client::addRequest(void) {

    Request *req = new Request(this);
    if (req == NULL) {
        Log.syserr() << "Cannot allocate memory for Request" << Log.endl;
        // shouldBeRemoved(true);
        return ;
    }
    _requests.push_back(req);

    if (++_nbRequests >= _maxRequests) {
        req->setStatus(TOO_MANY_REQUESTS);
        shouldBeClosed(true);
    }

    Log.debug() << "------------------------------------------" << Log.endl;
}

void Client::addResponse(void) {

    Request  *req = _requests.back();
    Response *res = new Response(req);

    if (res == NULL) {
        Log.syserr() << "Cannot allocate memory for Response" << Log.endl;
        // shouldBeRemoved(true);
        return ;
    }
    _responses.push_back(res);

    g_server->addToRespQ(_responses.back());

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

void Client::tryReplyResponse(int fd) {

    if (_responses.empty()) {
        return ;
    }

    HTTP::Response *res = _responses.front();
    if (res->formed() && !res->sent()) {
        reply(res);
    }

    if (res->formed() && res->sent()) {

        removeRequest();
        removeResponse();

        if (shouldBeClosed()) {
            g_server->unlink(fd);
            getClientIO()->reset();
        }
    }
}

void Client::tryReplyRequest(int fd) {

    if (_requests.empty()) {
        return ;
    }

    HTTP::Request *req = _requests.front();
    if (req->formed() && !req->sent()) {
        reply(req);
    }

    if (req->formed() && req->sent()) {
        setGatewayTimeout(Time::now());

        if (req->isCGI()) {
            Log.debug() << "Client:: request sent" << Log.endl; 
            g_server->unlink(fd);
            getGatewayIO()->wrFd(-1);
        }
    }
}

void Client::tryReceiveResponse(int fd) {

    if (_responses.empty()) {
        return ;
    }

    HTTP::Response *res = _responses.front();
    if (!res->formed()) {
        receive(res);
    }

    if (res->formed()) {
        if (!isTunnel()) {
            g_server->unlink(fd);
            getGatewayIO()->reset();
        }
    }
}

void Client::tryReceiveRequest(int fd) {
    (void)fd;
    if (_requests.size() == _responses.size()) {
        addRequest();
    }

    if (_requests.size() > _responses.size()) {
        receive(_requests.back());
    }

    if (_requests.back()->formed()) {
        addResponse();
    }
}

void
Client::checkTimeout(void) {

    std::time_t current = Time::now();

    if (getClientTimeout() != 0 && current - getClientTimeout() > g_server->settings.max_client_timeout) {
        
        IO *io = getClientIO();

        if (io->rdFd() >= 0) {
            Log.debug() << "Client:: [" << io->rdFd() << "] client timeout exceeded" << Log.endl;
            
            g_server->unlink(io->rdFd());
            io->reset();
            setClientTimeout(0);
        }
        // 408 Request Timeout
    }

    if (getGatewayTimeout() != 0 && current - getGatewayTimeout() > g_server->settings.max_gateway_timeout) {

        IO *io = getGatewayIO();
    
        if (io->rdFd() >= 0) {
            Log.debug() << "Client:: [" << io->rdFd() << "] gateway timeout exceeded" << Log.endl;
            
            g_server->unlink(io->rdFd());
            g_server->unlink(io->wrFd());
            setGatewayTimeout(0);
            io->reset();
            
            Response *res = _responses.front();

            if (res->isCGI() && res->getCGI()->getPID() != -1) {
                kill(res->getCGI()->getPID(), SIGKILL);
                res->getCGI()->setPID(-1);
            }
            // 504 Gateway Timeout
        }
    }
}

void Client::reply(Response *res) {
    signal(SIGPIPE, SIG_IGN);

    IO *io = getClientIO();

    if (!res->headSent()) {
        if (res->getHead().empty()) {
            res->headSent(true);
            return ;
        }

        if (!io->getDataPos()) {
            io->setData(res->getHead());
        }

    } else if (!res->bodySent()) {

        if (res->chunked()) {
            if (!io->getDataPos()) {
                io->setData(res->makeChunk());
            }
        } else {
            if (res->getBody().empty()) {
                res->bodySent(true);
                return ;
            }

            if (!io->getDataPos()) {
                io->setData(res->getBody());
            }
        }
    }

    int bytes = io->write();
    
    if (bytes < 0) {
        // check for blocking io
        return ;
    }
    
    if (bytes == 0) {
        g_server->unlink(io->wrFd());
        io->reset();

        g_server->unlink(getGatewayIO()->rdFd());
        g_server->unlink(getGatewayIO()->wrFd());
        getGatewayIO()->reset();
        return ;
    }

    if (static_cast<std::size_t>(bytes) >= io->getDataSize()) {
        if (!res->headSent()) {
            res->headSent(true);

        } else if (!res->bodySent() && !res->chunked()) {
            res->bodySent(true);
        }
    }
}

void Client::reply(Request *req) {

    signal(SIGPIPE, SIG_IGN);

    IO *io = getGatewayIO();

    if (!req->headSent()) {
        if (req->getHead().empty()) {
            req->headSent(true);
            return ;
        }

        if (!io->getDataPos()) {
            io->setData(req->getHead());
        }

    } else if (!req->bodySent()) {
        if (req->getBody().empty()) {
            req->bodySent(true);
            return ;
        }

        if (!io->getDataPos()) {
            io->setData(req->getBody());
        }
    }

    int bytes = io->write();
    
    if (bytes < 0) {
        // check for blocking io
        return ;
    }
    
    if (bytes == 0) {
        Log.debug() << "Client:: [" << io->wrFd() << "] peer closed connection" << Log.endl;
        g_server->unlink(io->wrFd());
        g_server->unlink(io->rdFd());
        io->reset();
        return ;
    }

    if (static_cast<std::size_t>(bytes) >= io->getDataSize()) {
        if (!req->headSent()) {
            req->headSent(true);
        } else if (!req->bodySent()) {
            req->bodySent(true);
        }
    }
}

void Client::receive(Request *req) {

    int bytes = getClientIO()->read();

    if (bytes < 0) {
        return ;

    } else if (bytes == 0) {
        Log.debug() << "Client::receive [" << getClientIO()->rdFd() << "] peer closed connection" << Log.endl;
        g_server->unlink(getClientIO()->rdFd());
        getClientIO()->reset();
        return ;
    }

    setClientTimeout(Time::now());

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
        if (res->isCGI()) {
            Log.debug() << "Client::receive CGI failed" << Log.endl;
            res->checkCGIFailure();

            g_server->unlink(getGatewayIO()->rdFd());
            getGatewayIO()->reset();
            setGatewayTimeout(0);
        }
        return ;

    } else if (bytes == 0) {
        Log.debug() << "Client::receive [" << getGatewayIO()->rdFd() << "] resp done" << Log.endl;

        if (res->isCGI()) {
            res->checkCGIFailure();

            // If no content-length header returned from CGI
            // read the whole body at once
            res->setBodySize(getGatewayIO()->getRem().length());
            Log.debug() << "Rem len: " << res->getBodySize() << Log.endl;
        }

        g_server->unlink(getGatewayIO()->rdFd());
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
