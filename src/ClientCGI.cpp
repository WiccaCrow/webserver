#include "ClientCGI.hpp"
#include "Server.hpp"

namespace HTTP {

ClientCGI::ClientCGI() 
    : AClient()
    , _headSent(false)
    , _bodySent(false) {}

ClientCGI::~ClientCGI() {
    for (size_t i = 0; i < _responses.size(); i++) {
        if (_responses[i] != NULL) {
            delete _responses[i];
        }
    }
}

ClientCGI::ClientCGI(const ClientCGI &other) : AClient(other) {
    *this = other;
}

ClientCGI &
ClientCGI::operator=(const ClientCGI &other) {
    if (this != &other) {
        _responses      = other._responses;
        _clientIpAddr   = other._clientIpAddr;
        _clientPort     = other._clientPort;
    }
    return *this;
}

void
ClientCGI::setIpAddr(const std::string &ipaddr) {
    _clientIpAddr = ipaddr;
}

const std::string &
ClientCGI::getIpAddr(void) const {
    return _clientIpAddr;
}

void
ClientCGI::setPort(size_t port) {
    _clientPort = port;
}

size_t
ClientCGI::getPort(void) const {
    return _clientPort;
}

const std::string
ClientCGI::getHostname() const {
    return (_clientPort != 0 ? _clientIpAddr + ":" + sztos(_clientPort) : _clientIpAddr);
}

Request *
ClientCGI::getRequest() {
    return _req;
}

void
ClientCGI::setRequest(Request *req) {
    _req = req;
}


Response *
ClientCGI::getResponse() {
    return _responses.front();
}

void
ClientCGI::addRequest(void) {

    _req = new Request(this);
    if (_req == NULL) {
        Log.syserr() << "Cannot allocate memory for Request" << Log.endl;
        shouldBeClosed(true);
    }
    Log.debug() << "----------------------" << Log.endl;
}

void
ClientCGI::addResponse(Response *res) {
    _responses.push_back(res);
    Log.debug() << "ClientCGI::addResponse " << res->getRequest()->getUriRef()._path << Log.endl;
}

void
ClientCGI::removeResponse(void) {
    delete _responses.front();
    _responses.pop_front();
}

bool
ClientCGI::validSocket(void) {
    return getWriteFd() != -1 && getReadFd() != -1 && !shouldBeClosed();
}

bool
ClientCGI::requestReady(void) {
    return validSocket() && getRequest() && getRequest()->isFormed();
}

bool
ClientCGI::replyReady(void) {
    return validSocket() && _responses.size() && getResponse()->isFormed();
}

bool
ClientCGI::replyDone(void) {
    return _headSent && _bodySent;
}

void
ClientCGI::replyDone(bool val) {
    _headSent = val;
    _bodySent = val;
}

void
ClientCGI::pollin(void) {

}

void
ClientCGI::pollout(void) {
    if (replyReady() && !replyDone()) {
        reply();
    }
    
    if (replyDone()) {
        removeResponse();
        replyDone(false);
    }
}

void
ClientCGI::pollhup(void) {
    shouldBeClosed(true);
    Log.syserr() << "Client::pollhup " << getReadFd() << Log.endl;
}

void
ClientCGI::pollerr(void) {
    shouldBeClosed(true);
    Log.syserr() << "Client::pollerr " << getReadFd() << Log.endl;
}

void
ClientCGI::reply(void) {

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
}

void
ClientCGI::receive(void) {

    if (getReadFd() < 0) {
        return;
    }

    if (!readSocket()) {
        shouldBeClosed(true);
        return ;
    }

    Log.debug() << "ClientCGI::receive [" << getReadFd() << "]" << Log.endl;

    while (!getRequest()->isFormed()) {
        std::string line;

        if (!getline(line, getRequest()->getBodySize())) {
            return ;
        } 
        getRequest()->parseLine(line);
    }
}

}
