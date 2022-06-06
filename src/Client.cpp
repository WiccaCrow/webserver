#include "Client.hpp"

namespace HTTP {

ReadSock Client::_reader;

Client::Client()
    : _fd(-1)
    , _clientPort(0)
    , _requestFormed(false)
    , _responseFormed(true) { }

Client::~Client() {
}

Client::Client(const Client &client) {
    *this = client;
}

Client &
Client::operator=(const Client &client) {
    if (this != &client) {
        _req       = client._req;
        _res       = client._res;
        _servBlock = client._servBlock;
        // Not think it is correct
        _fd             = client._fd;
        _ipAddr         = client._ipAddr;
        _clientPort     = client._clientPort;
        _serverPort     = client._serverPort;
        _requestFormed  = client._requestFormed;
        _responseFormed = client._responseFormed;
    }
    return *this;
}

void
Client::linkRequest(void) {
    _req.setClient(this);
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

bool
Client::isRequestFormed() const {
    return _requestFormed;
}

void
Client::setRequestFormed(bool formed) {
    _requestFormed = formed;
}

bool
Client::isResponseFormed() const {
    return _responseFormed;
}

void
Client::setResponseFormed(bool formed) {
    _responseFormed = formed;
}

void
Client::receive(void) {

    std::string line;

    struct s_sock s = { _fd, ReadSock::PERM_READ };

    Log.debug("Client::receive -> fd:" + to_string(_fd));

    ReadSock::Status stat;
    while (true) {
        line = "";
        if (!(_req.getFlags() & PARSED_HEADERS) || !(_req.getFlags() & PARSED_SL)) {
            stat = _reader.getline(s, line);
        } else {
            stat = _reader.getline_for_chunked(s, line, _req);
        }
        switch (stat) {
            case ReadSock::RECV_END:
                Log.debug("Client::recv_env " + to_string(_fd));
                setFd(-1);
                // need to erase from map of clients in server
            case ReadSock::INVALID_FD:
                return;
            case ReadSock::LINE_NOT_FOUND:
                return;

            case ReadSock::RECV_END_NB: {
                // Need to parse maybe
                // because the data could remain from the previous requests
                return;
            }

            case ReadSock::LINE_FOUND: {
                _req.parseLine(line);
                if (_req.getStatus() != HTTP::CONTINUE) {
                    setRequestFormed(true);
                    // _res.setFormed(true);
                    // _req.setFormed(true);
                    return;
                }
                break;
            }
            default: {
                return;
            }
        }
    }
}

void
Client::checkIfFailed(void) {

    if (_req.getStatus() == HTTP::BAD_REQUEST || _req.getStatus() == HTTP::REQUEST_TIMEOUT || _req.getStatus() == HTTP::INTERNAL_SERVER_ERROR || _req.getStatus() == HTTP::PAYLOAD_TOO_LARGE) {
        setFd(-1);
    }
}

void
Client::clearData(void) {
    _res.clear();
    _req.clear();
    _requestFormed  = false;
    _responseFormed = true;
}

void
Client::process(void) {
    if (_fd == -1) {
        return;
    }

    Log.debug("Client::process -> fd: " + to_string(_fd));

    HTTP::StatusCode status = _req.getStatus();

    if (status == HTTP::PROCESSING) {
        status = HTTP::OK;
    }

    if (status >= HTTP::BAD_REQUEST) {
        _res.setErrorResponse(status);
    } else if (status == HTTP::OK) {
        if (_res.handle(_req) == METHOD_NOT_ALLOWED) {
            _res.setErrorResponse(status = HTTP::METHOD_NOT_ALLOWED);
        }
    }
}

void
Client::reply(void) {
    if (_fd == -1) {
        return;
    }

    Log.debug("Client::reply -> fd: " + to_string(_fd));

    long sentBytes = 0;
    do {
        _res.setLeftToSend(sentBytes);
        sentBytes += send(_fd, _res.getLeftToSend(), _res.getLeftToSendSize(), 0);
        if (sentBytes < 0) {
            _req.setStatus(INTERNAL_SERVER_ERROR);
            break;
        }
    } while (static_cast<size_t>(sentBytes) < _res.getResLength());

    if (_res.shouldBeClosed()) {
        _fd = -1;
    }
}

}
