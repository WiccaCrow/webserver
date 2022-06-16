#include "Client.hpp"
#include "Server.hpp"

namespace HTTP {

Client::Client()
    : _fd(-1)
    , _clientPort(0)
    , _serverPort(0)
    , _shouldBeClosed(false) { }

Client::~Client() {
}

Client::Client(const Client &client) {
    *this = client;
}

Client &
Client::operator=(const Client &client) {
    if (this != &client) {
        _req            = client._req;
        _res            = client._res;
        _fd             = client._fd;
        _clientIpAddr   = client._clientIpAddr;
        _serverIpAddr   = client._serverIpAddr;
        _clientPort     = client._clientPort;
        _serverPort     = client._serverPort;
        _shouldBeClosed = client._shouldBeClosed;
    }
    return *this;
}

void
Client::initResponseMethodsHeaders(void) {
    _res.initMethodsHeaders();
}

void
Client::linkRequest(void) {
    _req.setClient(this);
    _res.setClient(this);
    _res.setRequest(&_req);
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
    return (_clientPort != 0 ? _clientIpAddr + ":" + to_string(_clientPort) : _clientIpAddr);
}

const Request &
Client::getRequest() const {
    return _req;
}

const Response &
Client::getResponse() const {
    return _res;
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
Client::checkIfFailed(void) {

    static const size_t size = 4;
    static const StatusCode failedStatuses[size] = {
        BAD_REQUEST,
        REQUEST_TIMEOUT,
        INTERNAL_SERVER_ERROR,
        PAYLOAD_TOO_LARGE
    };

    for (size_t i = 0; i < size; i++) {
        if (_req.getStatus() == failedStatuses[i]) {
            setFd(-1);
            return ;
        } 
    }
}

void
Client::clearData(void) {
    _res.clear();
    _req.clear();
}

void
Client::process(void) {
    if (_fd == -1) {
        return;
    }

    Log.debug("Client::process -> fd: " + to_string(_fd));

    if (_req.getStatus() == HTTP::PROCESSING) {
        _req.setStatus(HTTP::OK);
    }

    _res.handle();
}

void
Client::reply(void) {
    if (_fd == -1) {
        return;
    }

    Log.debug("Client::reply -> fd: " + to_string(_fd));
    // Log.debug("\n" + std::string(_res.getResponse()) + "\n");

    for (size_t total = _res.getResponseLength(); total;
                total = _res.getResponseLength()) { // пока есть что отправлять
        const char *rsp = _res.getResponse().c_str();

        size_t sent = 0;
        do {
            long n = send(_fd, rsp + sent, total - sent, 0);
            if (n > 0) {
                sent += n;
            }
            else if (n == 0) {
                setFd(-1);
                break ;
            }

        } while (sent < total);

        Log.debug("Client::send (" + to_string(sent) + "/" + to_string(total) + " bytes)");

        _res.makeChunk();
    }


    if (shouldBeClosed()) {
        _fd = -1;
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
    if (!_req.getBodySize()) {
        pos = _rem.find("\r\n");
        if (pos == std::string::npos) {
            return LINE_NOT_FOUND;
        }
        pos += 2;
    } else {
        if (_rem.length() < _req.getBodySize()) {
            return LINE_NOT_FOUND;
        }
        pos = _req.getBodySize();
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

    Log.debug("Client::receive [" + to_string(_fd) + "]");

    while (true) {
        std::string line;

        switch (getline(line)) {
            case LINE_FOUND: {
                _req.parseLine(line);
                if (!_req.isFormed()) {
                    break ;
                }
                return ;
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
    Log.debug("Client::matchServerBlock -> " + found->getBlockName() + " for " + host + ":" + to_string(_serverPort));
    return &(*found);
}

}
