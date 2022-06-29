#include "Client.hpp"
#include "Server.hpp"

namespace HTTP {

Client::Client() 
    : AClient()
    , _headSent(false)
    , _bodySent(false) {}

Client::~Client() {
    for (size_t i = 0; i < _responses.size(); i++) {
        if (_responses[i] != NULL) {
            delete _responses[i];
        }
    }
}

Client::Client(const Client &other) : AClient(other) {
    *this = other;
}

Client &
Client::operator=(const Client &other) {
    if (this != &other) {
        _responses      = other._responses;
        _clientIpAddr   = other._clientIpAddr;
        _clientPort     = other._clientPort;
    }
    return *this;
}

void
Client::setIpAddr(const std::string &ipaddr) {
    _clientIpAddr = ipaddr;
}

const std::string &
Client::getIpAddr(void) const {
    return _clientIpAddr;
}

void
Client::setPort(size_t port) {
    _clientPort = port;
}

size_t
Client::getPort(void) const {
    return _clientPort;
}

const std::string
Client::getHostname() const {
    return (_clientPort != 0 ? _clientIpAddr + ":" + sztos(_clientPort) : _clientIpAddr);
}

Request *
Client::getRequest() {
    return _req;
}

void
Client::setRequest(Request *req) {
    _req = req;
}


Response *
Client::getResponse() {
    return _responses.front();
}

void
Client::addRequest(void) {

    _req = new Request(this);
    if (_req == NULL) {
        Log.syserr() << "Cannot allocate memory for Request" << Log.endl;
        shouldBeClosed(true);
    }
    Log.debug() << "----------------------" << Log.endl;
}

void
Client::addResponse(Response *res) {
    _responses.push_back(res);
    Log.debug() << "Client::addResponse " << res->getRequest()->getUriRef()._path << Log.endl;
}

void
Client::removeResponse(void) {
    delete _responses.front();
    _responses.pop_front();
}

bool
Client::validSocket(void) {
    return getWriteFd() != -1 && getReadFd() != -1 && !shouldBeClosed();
}

bool
Client::requestReady(void) {
    return validSocket() && getRequest() && getRequest()->isFormed();
}

bool
Client::replyReady(void) {
    return validSocket() && _responses.size() && getResponse()->isFormed();
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
Client::pollin(void) {

    if (!getRequest()) {
        addRequest();
    }

    if (!requestReady()) {
        receive();
    }

    if (requestReady()) {
        g_server->requests.push_back(getRequest());
        setRequest(NULL);
    }
}

void
Client::pollout(void) {

    if (replyReady() && !replyDone()) {
        reply();
    }
    
    if (replyDone()) {
        removeResponse();
        replyDone(false);
    }

}

void
Client::pollhup(void) {
    shouldBeClosed(true);
    Log.syserr() << "Client::pollhup " << getReadFd() << Log.endl;
}


void
Client::pollerr(void) {
    shouldBeClosed(true);
    Log.syserr() << "Client::pollerr " << getReadFd() << Log.endl;
}

void
Client::reply(void) {

    signal(SIGPIPE, SIG_IGN);

    Response *rsp = getResponse();

    if (!_headSent) {
        if (!getDataPos()) {
            setData(rsp->getHead().c_str());
            setDataSize(rsp->getHead().length());
        }
        _headSent = sendData();

    } else if (!_bodySent) {
        if (!getDataPos()) {
            setData(rsp->getBody().c_str());
            setDataSize(rsp->getBody().length());
        }
        _bodySent = sendData();
    }

    if (replyDone()) {
        setData(NULL);
        setDataPos(0);
        setDataSize(0);
    }

    // if (shouldBeClosed()) {
    //     Log.debug() << "Client::shouldBeClosed" << Log.endl;
    //     setReadFd(-1);
    //     setWriteFd(-1);
    // }
}

void
Client::receive(void) {

    if (getReadFd() < 0) {
        return;
    }

    if (!readSocket()) {
        shouldBeClosed(true);
        return ;
    }

    Log.debug() << "Client::receive [" << getReadFd() << "]" << Log.endl;

    while (!getRequest()->isFormed()) {
        std::string line;

        if (!getline(line, getRequest()->getBodySize())) {
            return ;
        } 
        getRequest()->parseLine(line);
    }
}

}
