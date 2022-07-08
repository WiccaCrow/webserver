#include "IO.hpp"

static const std::size_t BUFFER_SIZE = 65536;

IO::IO(void) 
    : _fdr(-1)
    , _fdw(-1)
    , _af(AF_UNSPEC)
    , _port(0)
    , _data(0)
    , _dataSize(0)
    , _dataPos(0) {}

IO::~IO(void) {
    
}

int
IO::rdFd(void) const { 
    return _fdr;
}

int
IO::wrFd(void) const { 
    return _fdw;
}

void
IO::rdFd(int fd) {
    _fdr = fd;
}

void
IO::wrFd(int fd) {
    _fdw = fd;
}

void
IO::setFd(int fd) {
    rdFd(fd);
    wrFd(fd);
}

void
IO::setAddr(const std::string &addr) {
    _addr = addr;
}

std::size_t
IO::getPort(void) const {
    return _port;
}

void
IO::setPort(std::size_t port) {
    _port = port;
}

const std::string &
IO::getAddr(void) const {
    return _addr;
}

void
IO::setData(const char *data) {
    _data = data;
}

void
IO::setDataSize(std::size_t size) {
    _dataSize = size;
}

void
IO::setDataPos(std::size_t pos) {
    _dataPos = pos;
}

const char *
IO::getData(void) const {
    return _data;
}

std::size_t
IO::getDataSize(void) const {
    return _dataSize;
}

std::size_t
IO::getDataPos(void) const {
    return _dataPos;
}

void
IO::clear(void) {
    setData(NULL);
    setDataPos(0);
    setDataSize(0);
}

void
IO::closeRdFd(void) {
    if (_fdr != -1) {
        close(_fdr);
        rdFd(-1);
    }
}

void
IO::closeWrFd(void) {
    if (_fdw != -1) {
        close(_fdw);
        wrFd(-1);
    }
}

void
IO::closeFd(void) {
    if (_fdw != _fdr) {
        Log.error() << "IO::closeFd mismatch: " << _fdr << " " << _fdw << Log.endl;
        return ;
    }

    if (_fdr != -1) {
        close(_fdr);
        rdFd(-1);
        wrFd(-1);
    }
}

int
IO::pipe(void) {
    int tmp[2] = { -1 };

    if (::pipe(tmp) != 0) {
        Log.syserr() << "IO::pipe failed" << Log.endl;
        return -1;
    }
    rdFd(tmp[0]);
    wrFd(tmp[1]);
    return 0;
}

int
IO::create(int af) {
    int fd = socket(af, SOCK_STREAM, 0);
    if (fd < 0) {
        Log.syserr() << "IO::socket failed" << Log.endl;
    } else {
        _af = af;
        setFd(fd);
    }
    return fd;
}

int
IO::connect(const sockaddr *addr, socklen_t len) {

    if (_fdr != _fdw) {
        Log.error() << "IO::connect fdr and fdw differs" << Log.endl;
        return -1;
    }

    if (::connect(_fdw, addr, len) < 0) {
        return -1;
    }
    return _fdw;
}

int
IO::nonblock(void) {
    if (fcntl(_fdr, F_SETFL, O_NONBLOCK) < 0) {
        Log.syserr() << "IO::fcntl(O_NONBLOCK) failed [" << _fdr << "]" << Log.endl;
        return -1;
    }
    return 0;
}

int
IO::listen(const std::string &addr, std::size_t port) {

    int i = 1;
    struct sockaddr_in data;
    data.sin_family = _af;
    data.sin_port   = htons(port);
    data.sin_addr.s_addr = inet_addr(addr.c_str());

    setAddr(addr);
    setPort(port);
    if (::setsockopt(_fdr, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0) {
        Log.syserr() << "IO::setsockopt [" << _fdr << "] ->" << _addr << ":" << _port << Log.endl;
        return -1;
    }
    if (::bind(_fdr, (struct sockaddr *)&data, sizeof(data)) < 0) {
        Log.syserr() << "IO::bind [" << _fdr << "] ->" << _addr << ":" << _port << Log.endl;
        return -1;
    }
    if (::listen(_fdr, SOMAXCONN) < 0) {
        Log.syserr() << "IO::listen [" << _fdr << "] ->" << _addr << ":" << _port << Log.endl;
        return -1;
    }

    Log.info() << "IO::listen [" << _fdr << "] -> " << addr << ":" << port << Log.endl;
    return _fdr;
}

int IO::read(void) {

    char buf[BUFFER_SIZE + 1] = { 0 };

    int bytes = ::read(_fdr, buf, BUFFER_SIZE);
 
    if (bytes > 0) {
        buf[bytes] = '\0';    
        _rem.append(buf, bytes);
    } 

    // if (_rem.find("\x06") != std::string::npos) {
    //     return 0;
    // }

    return bytes;
}

int
IO::write(void) {

    long bytes = ::write(_fdw,  _data + _dataPos, _dataSize - _dataPos);
    
    if (bytes > 0) {
        _dataPos += bytes;
        Log.debug() << "IO::write [" << _fdw << "]: " << _dataPos << "/" << _dataSize << " bytes" << Log.endl;
    
        if (_dataPos >= _dataSize) {
            clear();
        }
    }

    return bytes;
}

int
IO::getline(std::string &line, std::size_t size) {
    std::size_t pos = 0;
    if (!size) {
        pos = _rem.find(LF);
        if (pos == std::string::npos) {
            return 0;
        }
        pos += 1;

    } else {
        if (_rem.length() < size) {
            // Log.debug() << _rem.length() << " " << size << Log.endl;
            return 0;
        }
        pos = size;
    }

    line = _rem.substr(0, pos);
    _rem.erase(0, pos);
    return 1;
}
