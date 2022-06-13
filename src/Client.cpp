#include "Client.hpp"

namespace HTTP {

Client::Client()
    : _fd(-1)
    , _clientPort(0)
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
        // _servBlock      = client._servBlock;
        _fd             = client._fd;
        _ipAddr         = client._ipAddr;
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
Client::setIpAddr(const std::string ipaddr) {
    _ipAddr = ipaddr;
}

void
Client::setPort(int port) {
    _clientPort = port;
}

int
Client::getPort(void) const {
    return _clientPort;
}

void
Client::setServerPort(int port) {
    _serverPort = port;
}

int
Client::getServerPort(void) const {
    return _serverPort;
}

const std::string &
Client::getIpAddr(void) const {
    return _ipAddr;
}

const std::string
Client::getHostname() const {
    return _clientPort != 0 ? _ipAddr + ":" + to_string(_clientPort) : _ipAddr;
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
    static const StatusCode failedStatus[size] = {
        BAD_REQUEST,
        REQUEST_TIMEOUT,
        INTERNAL_SERVER_ERROR,
        PAYLOAD_TOO_LARGE
    };

    for (size_t i = 0; i < size; i++) {
        if (_req.getStatus() == failedStatus[i]) {
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

    size_t sentBytes = 0;
    do {
        
        long n = send(_fd, _res.getResponse() + sentBytes, _res.getResLength() - sentBytes, 0);
        if (n > 0) {
            sentBytes += n;
            Log.debug("Client:: " + to_string(sentBytes) + "/" + to_string(_res.getResLength()) + " bytes sent");
        }
    } while (sentBytes < _res.getResLength());

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



}
