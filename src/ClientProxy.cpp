#include "ClientProxy.hpp"
#include "Server.hpp"

namespace HTTP {

ClientProxy::ClientProxy() 
    : AClient()
    , _headSent(false)
    , _bodySent(false) {}

ClientProxy::~ClientProxy() {
    for (size_t i = 0; i < _responses.size(); i++) {
        if (_responses[i] != NULL) {
            delete _responses[i];
        }
    }
}

ClientProxy::ClientProxy(const ClientProxy &other) : AClient(other) {
    *this = other;
}

ClientProxy &
ClientProxy::operator=(const ClientProxy &other) {
    if (this != &other) {
        _responses      = other._responses;
        _clientIpAddr   = other._clientIpAddr;
        _clientPort     = other._clientPort;
    }
    return *this;
}

void
ClientProxy::pollin(void) {

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
ClientProxy::pollout(void) {

}

void
ClientProxy::receive(void) {

    if (getReadFd() < 0) {
        return;
    }

    if (!readSocket()) {
        _res->makeResponseForError(BAD_GATEWAY);
        _res->isFormed(true);
        return ;
    }

    Log.debug() << "ClientProxy::receive [" << getReadFd() << "]" << Log.endl;

    while (!getRequest()->isFormed()) {
        std::string line;

        if (!getline(line, getRequest()->getBodySize())) {
            return ;
        } 
        getRequest()->parseLine(line);
    }
}

void
Client::setStatus(StatusCode status) {
    _status = status;
}

void
ClientProxy::setIpAddr(const std::string &ipaddr) {
    _clientIpAddr = ipaddr;
}

const std::string &
ClientProxy::getIpAddr(void) const {
    return _clientIpAddr;
}

void
ClientProxy::setPort(size_t port) {
    _clientPort = port;
}

size_t
ClientProxy::getPort(void) const {
    return _clientPort;
}

const std::string
ClientProxy::getHostname() const {
    return (_clientPort != 0 ? _clientIpAddr + ":" + sztos(_clientPort) : _clientIpAddr);
}

Request *
ClientProxy::getRequest() {
    return _req;
}

void
ClientProxy::setRequest(Request *req) {
    _req = req;
}


Response *
ClientProxy::getResponse() {
    return _responses.front();
}

void
ClientProxy::addRequest(void) {

    _req = new Request(this);
    if (_req == NULL) {
        Log.syserr() << "Cannot allocate memory for Request" << Log.endl;
        shouldBeClosed(true);
    }
    Log.debug() << "----------------------" << Log.endl;
}

void
ClientProxy::addResponse(Response *res) {
    _responses.push_back(res);
    Log.debug() << "ClientProxy::addResponse " << res->getRequest()->getUriRef()._path << Log.endl;
}

void
ClientProxy::removeResponse(void) {
    delete _responses.front();
    _responses.pop_front();
}

bool
ClientProxy::validSocket(void) {
    return getWriteFd() != -1 && getReadFd() != -1 && !shouldBeClosed();
}

bool
ClientProxy::requestReady(void) {
    return validSocket() && getRequest() && getRequest()->isFormed();
}

bool
ClientProxy::replyReady(void) {
    return validSocket() && _responses.size() && getResponse()->isFormed();
}

bool
ClientProxy::replyDone(void) {
    return _headSent && _bodySent;
}

void
ClientProxy::replyDone(bool val) {
    _headSent = val;
    _bodySent = val;
}

void
ClientProxy::reply(void) {

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
    //     Log.debug() << "ClientProxy::shouldBeClosed" << Log.endl;
    //     setReadFd(-1);
    //     setWriteFd(-1);
    // }
}

}
