#include "Client.hpp"

#include "Server.hpp"

namespace HTTP {

Client::Client(void)
    : _clientIO(NULL), _serverIO(NULL), _targetIO(NULL), _headSent(false), _bodySent(false), _shouldBeClosed(false), _shouldBeRemoved(false), _isTunnel(false), _nbRequests(0), _maxRequests(MAX_REQUESTS), _clientTimeout(0), _targetTimeout(0) {
    _clientIO = new IO();
    if (_clientIO == NULL) {
        Log.syserr() << "Client:: Cannot allocate memory for socket" << Log.endl;
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

    if (_targetIO) {
        delete _targetIO;
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
Client::getTargetTimeout(void) const {
    return _targetTimeout;
}

void Client::setClientTimeout(time_t time) {
    _clientTimeout = time;
}

void Client::setTargetTimeout(time_t time) {
    _targetTimeout = time;
}

IO *Client::getClientIO(void) {
    return _clientIO;
}

IO *Client::getServerIO(void) {
    return _serverIO;
}

IO *Client::getTargetIO(void) {
    return _targetIO;
}

void Client::setClientIO(IO *sock) {
    _clientIO = sock;
}

void Client::setServerIO(IO *sock) {
    _serverIO = sock;
}

void Client::setTargetIO(IO *sock) {
    _targetIO = sock;
}

const std::string
Client::getHostname(void) {
    const std::size_t  port = getClientIO()->getPort();
    const std::string &addr = getClientIO()->getAddr();

    return (port != 0 ? addr + ":" + sztos(port) : addr);
}

bool Client::replyDone(void) {
    return _headSent && _bodySent;
}

void Client::replyDone(bool val) {
    _headSent = val;
    _bodySent = val;
}

void Client::addRequest(void) {
    Request *req = new Request(this);
    if (req == NULL) {
        Log.syserr() << "Cannot allocate memory for Request" << Log.endl;
        shouldBeClosed(true);
        return;
    }
    _requests.push_back(req);
    Log.debug() << "----------------------" << Log.endl;
}

void Client::addResponse(void) {
    Request  *req = _requests.back();
    Response *res = new Response(req);
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
            setClientTimeout(std::time(0));
            receive(_requests.back());

            if (_requests.back()->formed()) {
                _nbRequests++;
                addResponse();
                g_server->addToQueue(_responses.back());
            }
        }
    } else if (fd == getTargetIO()->rdFd()) {
        if (_responses.size() > 0) {
            receive(_responses.front());
        }
    }
}

void Client::pollout(int fd) {

    if (fd == getClientIO()->wrFd()) {
        if (_responses.empty()) {
            return;
        }

        if (_responses.front()->formed() && !replyDone()) {
            reply(_responses.front());
        }

        if (replyDone()) {
            if (_nbRequests >= _maxRequests) {
                shouldBeClosed(true);
            }

            removeResponse();
            removeRequest();
            getClientIO()->clear();

            if (getTargetIO()) {
                g_server->rmPollFd(getTargetIO()->rdFd());
                delete getTargetIO();
                setTargetIO(NULL);
            }
            replyDone(false);

            if (shouldBeClosed()) {
                shouldBeRemoved(true);
            }
        }
    }
    // else if (fd == getTargetIO()->wrFd()) {
    //     reply(_requests.front());
    // }
}

void Client::pollhup(int fd) {
    Log.syserr() << "Client::pollhup " << fd << Log.endl;
    if (fd == getClientIO()->rdFd()) {
        shouldBeRemoved(true);
    } else {
        g_server->rmPollFd(getTargetIO()->rdFd());
    }
}

void Client::pollerr(int fd) {
    Log.syserr() << "Client::pollerr " << fd << Log.endl;
    shouldBeClosed(true);
}

void Client::reply(Response *res) {
    signal(SIGPIPE, SIG_IGN);

    if (!_headSent) {
        if (!getClientIO()->getDataPos()) {
            getClientIO()->setData(res->getHead().c_str());
            getClientIO()->setDataSize(res->getHead().length());
        }
        _headSent = getClientIO()->write();

    } else if (!_bodySent) {
        if (!getClientIO()->getDataPos()) {
            getClientIO()->setData(res->getBody().c_str());
            getClientIO()->setDataSize(res->getBody().length());
        }
        _bodySent = getClientIO()->write();
    }
}

void Client::reply(Request *req) {
    signal(SIGPIPE, SIG_IGN);

    if (!_headSent) {
        if (!getTargetIO()->getDataPos()) {
            getTargetIO()->setData(req->getHead().c_str());
            getTargetIO()->setDataSize(req->getHead().length());
        }
        _headSent = getTargetIO()->write();

    } else if (!_bodySent) {
        if (!getTargetIO()->getDataPos()) {
            getTargetIO()->setData(req->getBody().c_str());
            getTargetIO()->setDataSize(req->getBody().length());
        }
        _bodySent = getTargetIO()->write();
    }
}

void Client::receive(Request *req) {
    Log.debug() << "Client:: [" << getClientIO()->rdFd() << "] receive request" << Log.endl;

    if (!getClientIO()->read()) {
        Log.debug() << "Client:: [" << getClientIO()->rdFd() << "] peer closed connection" << Log.endl;
        shouldBeRemoved(true);
        return;
    }

    while (!req->formed()) {
        std::string line;

        if (!getClientIO()->getline(line, req->getBodySize())) {
            return;
        }
        req->parseLine(line);
    }
}

void Client::receive(Response *res) {
    int bytes = getTargetIO()->read();
    if (bytes < 0) {
        Log.debug() << "Client:: [" << getTargetIO()->rdFd() << "] receive -1 response" << Log.endl;
        return;
    } else if (bytes == 0) {
        Log.debug() << "Client:: [" << getTargetIO()->rdFd() << "] receive 0 response" << Log.endl;

        if (res->isCGI()) {
            res->checkCGIFail();
            setTargetTimeout(0);
            g_server->rmPollFd(getTargetIO()->rdFd());
        }

        if (res->isProxy()) {
            setTargetTimeout(0);
            g_server->rmPollFd(getTargetIO()->rdFd());
        }
    }

    while (!res->formed()) {
        std::string line;

        if (res->getStatus() >= BAD_REQUEST) {
            res->assembleError();
        }

        if (!getTargetIO()->getline(line, res->getBodySize())) {
            return;
        }
        res->parseLine(line);
    }
}

ServerBlock *
Client::matchServerBlock(const std::string &host) {
    typedef std::list<HTTP::ServerBlock>::iterator iter_l;
    typedef std::list<HTTP::ServerBlock>           bslist;

    bool searchByName = !isValidIpv4(host) ? true : false;

    bslist &blocks = g_server->operator[](getServerIO()->getPort());
    iter_l                     found = blocks.end();
    for (iter_l block = blocks.begin(); block != blocks.end(); ++block) {
        if (block->hasAddr(getServerIO()->getAddr())) {
            if (found == blocks.end()) {
                found = block;
                if (!searchByName) {
                    break;
                }
            }
            if (searchByName && block->hasName(host)) {
                found = block;
                break;
            }
        }
    }
    Log.debug() << "Client:: servBlock " << found->getBlockName() << " for " << host << ":" << getServerIO()->getPort() << Log.endl;
    return &(*found);
}

} // namespace HTTP
