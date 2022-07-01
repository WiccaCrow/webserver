#include "Client.hpp"
#include "Server.hpp"

namespace HTTP {

Client::Client(void)
    : _clientSock(NULL)
    , _serverSock(NULL)
    , _targetSock(NULL)
    , _headSent(false)
    , _bodySent(false)
    , _shouldBeClosed(false)
    , _nbRequests(0)
    , _maxRequests(MAX_REQUESTS)
    , _clientTimeout(0) 
    , _targetTimeout(0) {

    _clientSock = new Socket();
    _targetSock = new Socket();

    if (_clientSock == NULL || _targetSock == NULL) {
        Log.syserr() << "Client::Cannot allocate memory for Sockets" << Log.endl;
    }
}

Client::~Client(void) {
    typedef std::list<Request *> PoolReq;
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
}

void
Client::shouldBeClosed(bool flag) {
    _shouldBeClosed = flag;
}

bool
Client::shouldBeClosed(void) const {
    return _shouldBeClosed;
}

time_t
Client::getClientTimeout(void) const {
    return _clientTimeout;
}

time_t
Client::getTargetTimeout(void) const {
    return _targetTimeout;
}

void
Client::setClientTimeout(time_t time) {
    _clientTimeout = time;
}

void
Client::setTargetTimeout(time_t time) {
    _targetTimeout = time;
}

Socket *
Client::getClientSock(void) {
    return _clientSock;
}

Socket *
Client::getServerSock(void) {
    return _serverSock;
}

Socket *
Client::getTargetSock(void) {
    return _targetSock;
}

void
Client::setClientSock(Socket *sock) {
    _clientSock = sock;
}

void
Client::setServerSock(Socket *sock) {
    _serverSock = sock;
}

void
Client::setTargetSock(Socket *sock) {
    _targetSock = sock;
}

const std::string
Client::getHostname(void) {
    const std::size_t port = getClientSock()->getPort();
    const std::string &addr = getClientSock()->getAddr();

    return (port != 0 ? addr + ":" + sztos(port) : addr);
}

bool
Client::replyDone(void) {
    return _headSent && _bodySent;
}

void
Client::replyDone(bool val) {
    _headSent = val;
    _bodySent = val;
}

void
Client::addRequest(void) {

    Request *req = new Request(this);
    if (req == NULL) {
        Log.syserr() << "Cannot allocate memory for Request" << Log.endl;
        shouldBeClosed(true);
        return ;
    }
    _requests.push_back(req);
    Log.debug() << "----------------------" << Log.endl;
}

void
Client::addResponse(void) {

    Request *req = _requests.back();
    Response *res = new Response(req);
    _responses.push_back(res);

    Log.debug() << "Client::addResponse " << res->getRequest()->getUriRef()._path << Log.endl;
}

void
Client::removeRequest(void) {
    if (_requests.size() > 0) {
        Request *req = _requests.front();
        _requests.pop_front();
        delete req;
    }
}

void
Client::removeResponse(void) {
    if (_responses.size() > 0) {
        Response *res = _responses.front();
        _responses.pop_front();
        delete res;
    }
}

void
Client::pollin(int fd) {

    if (fd == getClientSock()->getFd()) {

        if (_requests.size() == _responses.size()) {
            addRequest();
        }

        if (_requests.size() > _responses.size()) {
            setClientTimeout(std::time(0));
            receive(_requests.back());
            
            if (_requests.back()->formed()) {
                _nbRequests++;
                addResponse();
                g_server->responses.push_back(_responses.back());
            }
        }
    } 
    else if (fd == getTargetSock()->getFd()) {
        receive(_responses.front());
    }
 
}

void
Client::pollout(int fd) {
    (void)fd;

    if (fd == getClientSock()->getFd()) {

        if (_responses.empty()) {
            return ;
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
            getClientSock()->clear();
            replyDone(false);
        }
    }
    // else if (fd == getTargetSock()->getFd()) {
    //     reply(_requests.front());
    // }
}

void
Client::pollhup(int fd) {
    Log.syserr() << "Client::pollhup " << fd << Log.endl;
    shouldBeClosed(true);
}


void
Client::pollerr(int fd) {
    Log.syserr() << "Client::pollerr " << fd << Log.endl;
    shouldBeClosed(true);
}

void
Client::reply(Response *res) {

    signal(SIGPIPE, SIG_IGN);

    if (!_headSent) {
        if (!getClientSock()->getDataPos()) {
            getClientSock()->setData(res->getHead().c_str());
            getClientSock()->setDataSize(res->getHead().length());
        }
        _headSent = getClientSock()->write();

    } else if (!_bodySent) {
        if (!getClientSock()->getDataPos()) {
            getClientSock()->setData(res->getBody().c_str());
            getClientSock()->setDataSize(res->getBody().length());
        }
        _bodySent = getClientSock()->write();
    }
}

void
Client::reply(Request *req) {

    signal(SIGPIPE, SIG_IGN);

    // Not send SL and headers if cgi
    // if (isCGI) _headSent = true;
    // or make head empty (easier)

    if (!_headSent) {
        if (!getTargetSock()->getDataPos()) {
            getTargetSock()->setData(req->getHead().c_str());
            getTargetSock()->setDataSize(req->getHead().length());
        }
        _headSent = getTargetSock()->write();

    } else if (!_bodySent) {
        if (!getTargetSock()->getDataPos()) {
            getTargetSock()->setData(req->getBody().c_str());
            getTargetSock()->setDataSize(req->getBody().length());
        }
        _bodySent = getTargetSock()->write();
    }
}

void
Client::receive(Request *req) {

    Log.debug() << "Client:: [" << getClientSock()->getFd() << "] receive request" << Log.endl;
    
    if (!getClientSock()->read()) {
        Log.debug() << "Client:: [" << getClientSock()->getFd() << "] peer closed connection" << Log.endl;
        // g_server->rmPollFd(getClientSock()->getFd());
        // g_server->rmPollFd(getTargetSock()->getFd());
        shouldBeClosed(true);
        return ;
    }

    while (!req->formed()) {
        std::string line;

        if (!getClientSock()->getline(line, req->getBodySize())) {
            return ;
        }
        req->parseLine(line);
    }
}

void
Client::receive(Response *res) {

    Log.debug() << "Client:: [" << getTargetSock()->getFd() << "] receive response" << Log.endl;
    
    if (!getTargetSock()->read()) {
        if (res->isCGI()) {
            g_server->rmPollFd(getTargetSock()->getFd());
        }

        if (res->isProxy()) {
            g_server->rmPollFd(getTargetSock()->getFd());
        }
    }

    while (!res->formed()) {
        std::string line;

        if (!getTargetSock()->getline(line, res->getBodySize())) {
            return ;
        }
        res->parseLine(line);
    }
}

}
