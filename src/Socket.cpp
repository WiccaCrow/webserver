#include "Socket.hpp"

static const std::size_t BUFFER_SIZE = 65536;

Socket::Socket(void) 
    : _fd(-1)
    , _af(AF_UNSPEC)
    , _port(0)
    , _data(0)
    , _dataSize(0)
    , _dataPos(0) {}

Socket::~Socket(void) {
    if (_fd != -1) {
        close(_fd);
    }
}

int
Socket::getFd(void) const { 
    return _fd;
}

void
Socket::setFd(int fd) {
    _fd = fd;
}

void
Socket::setAddr(const std::string &addr) {
    _addr = addr;
}

std::size_t
Socket::getPort(void) const {
    return _port;
}

void
Socket::setPort(std::size_t port) {
    _port = port;
}

const std::string &
Socket::getAddr(void) const {
    return _addr;
}

void
Socket::setData(const char *data) {
    _data = data;
}

void
Socket::setDataSize(std::size_t size) {
    _dataSize = size;
}

void
Socket::setDataPos(std::size_t pos) {
    _dataPos = pos;
}

const char *
Socket::getData(void) const {
    return _data;
}

std::size_t
Socket::getDataSize(void) const {
    return _dataSize;
}

std::size_t
Socket::getDataPos(void) const {
    return _dataPos;
}

void
Socket::clear(void) {
    setData(NULL);
    setDataPos(0);
    setDataSize(0);
}

int
Socket::create(int af) {
    int fd = socket(af, SOCK_STREAM, 0);
    if (fd < 0) {
        Log.syserr() << "Server:: socket failed" << Log.endl;
    } else {
        _af = af;
        _fd = fd;
    }
    return fd;
}

int
Socket::connect(const sockaddr *addr, socklen_t len) {

    if (::connect(_fd, addr, len) < 0) {
        return -1;
    }
    return _fd;
}

int
Socket::nonblock(void) {
    if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0) {
        Log.syserr() << "Cannot set nonblock flag to " << _fd << Log.endl;
        return -1;
    }
    return 0;
}

int
Socket::listen(const std::string &addr, std::size_t port) {

    int i = 1;
    struct sockaddr_in data;
    data.sin_family = _af;
    data.sin_port   = htons(port);
    data.sin_addr.s_addr = inet_addr(addr.c_str());

    setAddr(addr);
    setPort(port);
    if (::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
        Log.syserr() << "Socket::setsockopt ->" << _addr << ":" << _port << Log.endl;
        return -1;
    }
    if (::bind(_fd, (struct sockaddr *)&data, sizeof(data)) < 0) {
        Log.syserr() << "Socket::bind ->" << _addr << ":" << _port << Log.endl;
        return -1;
    }
    if (::listen(_fd, SOMAXCONN) < 0) {
        Log.syserr() << "Socket::listen ->" << _addr << ":" << _port << Log.endl;
        return -1;
    }

    Log.info() << "Socket::listen [" << _fd << "] -> " << addr << ":" << port << Log.endl;
    return _fd;
}

int Socket::read(void) {

    char buf[BUFFER_SIZE + 1] = { 0 };

    int bytes = ::read(_fd, buf, BUFFER_SIZE);

    if (bytes == 0) {
        _rem.erase();

    } else if (bytes > 0) {
        buf[bytes] = '\0';    
        _rem += buf;
    } 
    
    if (_rem.find("\x06") != std::string::npos) {
        return 0;
    }
    return bytes;
}

int
Socket::write(void) {

    long sent = 0;

    do {
        sent = ::write(_fd,  _data + _dataPos, _dataSize - _dataPos);
        if (sent <= 0) {
            break ;
        }
        _dataPos += sent;
    } while (_dataPos < _dataSize);

    if (_dataPos < _dataSize) {
        return false;
    }

    Log.debug() << "Socket::write [" << _fd << "]: " << _dataPos << "/" << _dataSize << " bytes" << Log.endl;
    _dataPos = 0;
    return true;
}

int
Socket::getline(std::string &line, std::size_t size) {
    std::size_t pos = 0;
    if (!size) {
        pos = _rem.find(LF);
        if (pos == std::string::npos) {
            return 0;
        }
        pos += 1;

    } else {
        if (_rem.length() < size) {
            return 0;
        }
        pos = size;
    }

    line = _rem.substr(0, pos);
    _rem.erase(0, pos);
    return 1;
}
