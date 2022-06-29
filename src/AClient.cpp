#include "AClient.hpp"

namespace HTTP {

AClient::AClient(void) 
    : _fdr(-1)
    , _fdw(-1)
    , _serverPort(0)
    , _shouldBeClosed(false)
    , _data(NULL)
    , _dataSize(0)
    , _dataPos(0)
    {}

AClient::AClient(const AClient &other) {
    *this = other;
}

AClient&
AClient::operator=(const AClient &other) {
    if (this != &other) {
        _fdr            = other._fdr;
        _fdw            = other._fdw;
        _serverIpAddr   = other._serverIpAddr;
        _serverPort     = other._serverPort;
        _shouldBeClosed = other._shouldBeClosed;
        _data           = other._data;
        _dataSize       = other._dataSize;
        _dataPos        = other._dataPos;
        // _headSent       = other._headSent;
        // _bodySent       = other._bodySent;
    }
    return *this;
}

AClient::~AClient(void) {}

void
AClient::shouldBeClosed(bool flag) {
    _shouldBeClosed = flag;
}

bool
AClient::shouldBeClosed(void) const {
    return _shouldBeClosed;
}

int
AClient::getReadFd(void) const {
    return _fdr;
}

void
AClient::setReadFd(int fd) {
    _fdr = fd;
}

int
AClient::getWriteFd(void) const {
    return _fdw;
}

void
AClient::setWriteFd(int fd) {
    _fdw = fd;
}

void
AClient::setServerPort(size_t port) {
    _serverPort = port;
}

size_t
AClient::getServerPort(void) const {
    return _serverPort;
}

void
AClient::setServerIpAddr(const std::string &ipaddr) {
    _serverIpAddr = ipaddr;
}

const std::string &
AClient::getServerIpAddr(void) const {
    return _serverIpAddr;
}

void
AClient::setData(const char *data) {
    _data = data;
}

void
AClient::setDataSize(size_t size) {
    _dataSize = size;
}

void
AClient::setDataPos(size_t pos) {
    _dataPos = pos;
}


const char *
AClient::getData(void) const {
    return _data;
}

size_t
AClient::getDataSize(void) const {
    return _dataSize;
}

size_t
AClient::getDataPos(void) const {
    return _dataPos;
}

bool
AClient::sendData(void) {
    long sent = 0;

    do {
        sent = write(_fdw,  _data + _dataPos, _dataSize - _dataPos);
        // sent = send(_fdw, _data + _dataPos, _dataSize - _dataPos, 0);
        if (sent <= 0) {
            break ;
        }
        _dataPos += sent;
    } while (_dataPos < _dataSize);

    if (sent == 0) {
        // _headSent = true;
        // _bodySent = true;
        shouldBeClosed(true);
        Log.debug() << "Client::reply [" << _fdw << "] disconnect" << Log.endl;
    }

    if (_dataPos < _dataSize) {
        return false;
    }

    Log.debug() << "Client::reply [" << _fdw << "]: " << _dataPos << "/" << _dataSize << " bytes" << Log.endl;
    _dataPos = 0;
    return true;
}


static const size_t MAX_PACKET_SIZE = 65536;

int
AClient::readSocket(void) {
    char buf[MAX_PACKET_SIZE + 1] = { 0 };

    int recvBytes = read(_fdr, buf, MAX_PACKET_SIZE);
    // int recvBytes = recv(_fdr, buf, MAX_PACKET_SIZE, 0);

    if (recvBytes == 0) {
        _rem.erase();

    } else if (recvBytes > 0) {
        buf[recvBytes] = '\0';    
        _rem += buf;
    } 
    
    if (_rem.find("\x06") != std::string::npos) {
        return 0;
    }
    return recvBytes;
}

int
AClient::getline(std::string &line, size_t bodySize) {

    size_t pos = 0;
    if (!bodySize) {
        pos = _rem.find("\r\n");
        if (pos == std::string::npos) {
            return 0;
        }
        pos += 2;

    } else {
        if (_rem.length() < bodySize) {
            return 0;
        }
        pos = bodySize;
    }

    line = _rem.substr(0, pos);
    _rem.erase(0, pos);

    return 1;
}

}