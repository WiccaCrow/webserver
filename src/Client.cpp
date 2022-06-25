#include "Client.hpp"
#include "Server.hpp"

namespace HTTP {

Client::Client()
    : _fd(-1)
    , _clientPort(0)
    , _serverPort(0)
    , _req(NULL)
    , _shouldBeClosed(false)
    , _data(NULL)
    , _dataSize(0)
    , _dataPos(0)
    , _headSent(false)
    , _bodySent(false) {}

Client::~Client() {
    for (size_t i = 0; i < _responses.size(); i++) {
        if (_responses[i] != NULL) {
            delete _responses[i];
        }
    }
}

Client::Client(const Client &client) {
    *this = client;
}

Client &
Client::operator=(const Client &other) {
    if (this != &other) {
        _responses      = other._responses;
        _fd             = other._fd;
        _clientIpAddr   = other._clientIpAddr;
        _serverIpAddr   = other._serverIpAddr;
        _clientPort     = other._clientPort;
        _serverPort     = other._serverPort;
        _shouldBeClosed = other._shouldBeClosed;
        _data           = other._data;
        _dataSize       = other._dataSize;
        _dataPos        = other._dataPos;
        _headSent       = other._headSent;
        _bodySent       = other._bodySent;
    }
    return *this;
}

int
Client::getFd(void) const {
    return _fd;
}

void
Client::setFd(int fd) {
    _fd = fd;
}

void
Client::setIpAddr(const std::string &ipaddr) {
    _clientIpAddr = ipaddr;
}

void
Client::setServerIpAddr(const std::string &ipaddr) {
    _serverIpAddr = ipaddr;
}

const std::string &
Client::getServerIpAddr(void) const {
    return _serverIpAddr;
}

void
Client::setPort(size_t port) {
    _clientPort = port;
}

size_t
Client::getPort(void) const {
    return _clientPort;
}

void
Client::setServerPort(size_t port) {
    _serverPort = port;
}

size_t
Client::getServerPort(void) const {
    return _serverPort;
}

const std::string &
Client::getIpAddr(void) const {
    return _clientIpAddr;
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
Client::shouldBeClosed(bool flag) {
    _shouldBeClosed = flag;
}

bool
Client::shouldBeClosed(void) const {
    return _shouldBeClosed;
}

void
Client::addRequest(void) {

    _req = new Request(this);
    if (_req == NULL) {
        Log.syserr() << "Cannot allocate memory for Request" << Log.endl;
        setFd(-1);
    }
    Log.debug() << "----------------------" << Log.endl;
}

void
Client::addResponse(Response *res) {
    _responses.push_back(res);
}

void
Client::removeResponse(void) {
    delete _responses.front();
    _responses.pop_front();
}

bool
Client::validSocket(void) {
    return _fd != -1;
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

bool
Client::sendData(void) {
    long sent = 0;

    do {
        sent = send(_fd, _data + _dataPos, _dataSize - _dataPos, 0);
        if (sent <= 0) {
            Log.debug() << "Client::reply [" << _fd << "]: " << sent << Log.endl;
            break ;
        }
        _dataPos += sent;

        Log.debug() << "Client::reply [" << _fd << "]: ";
        Log << _dataPos << "/" << _dataSize << " bytes" << Log.endl;

    } while (_dataPos < _dataSize);

    if (sent == 0) {
        _headSent = true;
        _bodySent = true;
        shouldBeClosed(true);
    }

    if (_dataPos < _dataSize) {
        return false;
    }

    _dataPos = 0;
    return true;
}

void
Client::reply(void) {

    signal(SIGPIPE, SIG_IGN);

    Response *rsp = getResponse();

    if (!_headSent) {
        if (_dataPos == 0) {
            _data = rsp->getHead().c_str();
            _dataSize = rsp->getHead().length();
        }
        _headSent = sendData();

    } else if (!_bodySent) {
        if (_dataPos == 0) {
            _data = rsp->getBody().c_str();
            _dataSize = rsp->getBody().length();
        }
        _bodySent = sendData();
    }

    if (replyDone()) {
        _data = NULL;
        _dataPos = 0;
        _dataSize = 0;
    }

    if (shouldBeClosed()) {
        Log.debug() << "Client::shouldBeClosed" << Log.endl;
        setFd(-1);
    }
    
}

static const size_t MAX_PACKET_SIZE = 65536;

int
Client::readSocket(void) {
    char buf[MAX_PACKET_SIZE + 1] = { 0 };

    int recvBytes = recv(_fd, buf, MAX_PACKET_SIZE, 0);

    if (recvBytes == 0) {
        _rem.erase();

    } else if (recvBytes > 0) {
        buf[recvBytes] = '\0';
        _rem += buf;
    }
    return recvBytes;
}

Client::Status
Client::getline(std::string &line) {

    if (!readSocket()) {
        return SOCK_CLOSED;
    }

    size_t pos = 0;
    const size_t bodySize = getRequest()->getBodySize();
    if (!bodySize) {
        pos = _rem.find("\r\n");
        if (pos == std::string::npos) {
            return LINE_NOT_FOUND;
        }
        pos += 2;
    } else {
        if (_rem.length() < bodySize) {
            return LINE_NOT_FOUND;
        }
        pos = bodySize;
    }

    line = _rem.substr(0, pos);
    _rem.erase(0, pos);

    return LINE_FOUND;
}

void
Client::receive(void) {

    if (_fd < 0) {
        return;
    }
    Log.debug() << "Client::receive [" << _fd << "]" << Log.endl;

    while (!getRequest()->isFormed()) {
        std::string line;

        switch (getline(line)) {
            case LINE_FOUND: {
                getRequest()->parseLine(line);
                break ;
            }
            case SOCK_CLOSED: {
                setFd(-1);
                return ;
            }
            case LINE_NOT_FOUND: {
                return ;
            }
        }
    }
}

HTTP::ServerBlock *
Client::matchServerBlock(const std::string &host) const {
    typedef std::list<HTTP::ServerBlock>::iterator iter_l;
    typedef std::list<HTTP::ServerBlock> bslist;

    bool searchByName = !isValidIpv4(host) ? true : false;

    bslist &blocks = g_server->getServerBlocks(_serverPort);
    iter_l found = blocks.end();
    for (iter_l block = blocks.begin(); block != blocks.end(); ++block) {
        if (block->hasAddr(getServerIpAddr())) {
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
    Log.debug() << "Client:: servBlock " << found->getBlockName() << " for " << host << ":" << _serverPort << Log.endl;
    return &(*found);
}

}
