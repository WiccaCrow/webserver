#include "ReadSock.hpp"

static const size_t MAX_PACKET_SIZE = 65536;

#include <iostream>

ReadSock::Status ReadSock::readSocket(int fd) {
    char buf[MAX_PACKET_SIZE + 1] = {0};

    int recvBytes = recv(fd, buf, MAX_PACKET_SIZE, 0);

    if (recvBytes < 0) {
        //_rems.erase(fd); // Not needed with nonblocking sockets
        return RECV_END_NB;

    } else if (recvBytes == 0) {
        _rems.erase(fd);
        return RECV_END;

    } else {
        buf[recvBytes + 1] = '\0';
        _rems[fd] += buf;
        return RECV_DONE;
    }
}

int ReadSock::readSocket_for_chunked(struct s_sock &sock, int fd) {
    if (sock.perm & PERM_READ) {
        ReadSock::Status status = readSocket(fd);
        if (status == ReadSock::RECV_END) {
            return 0; // ???
        }
        if (status == ReadSock::RECV_DONE) {
            sock.perm |= ~PERM_READ;
        }
    }
    return (1);
}

ReadSock::Status ReadSock::getline_for_chunked(struct s_sock &sock, std::string &line,
                                               HTTP::Request &req) {
    int fd = sock.fd;
    if (fd < 0) {
        return INVALID_FD;
    }

    if (req.getChunked_isSizeChunk()) {
        if (!readSocket_for_chunked(sock, fd)) {
            return ReadSock::RECV_END;
        }
        size_t pos = _rems[fd].find("\r\n");
        if (pos == std::string::npos) {
            req.setStatus(HTTP::BAD_REQUEST);
            _rems[fd].clear();
            return LINE_NOT_FOUND;
        }
        line = _rems[fd].substr(0, pos);
        _rems[fd].erase(0, pos + 2);
        return LINE_FOUND;
        
    } else {
        long chunkSize = req.getChunked_Size();
        size_t remsLength = _rems[fd].length();
        while (remsLength < chunkSize + 2) {
            if (!readSocket_for_chunked(sock, fd)) {
                return ReadSock::RECV_END;
            }
            remsLength = _rems[fd].length();
        }
        if (_rems[fd][chunkSize] != '\r' &&
            _rems[fd][chunkSize + 1] != '\n') {
            line = _rems[fd].substr(0, chunkSize + 2);
            _rems[fd].clear();
        } else {
            line = _rems[fd].substr(0, chunkSize);
            _rems[fd].erase(0, chunkSize + 2);
        }
    }

    return LINE_FOUND;
}

ReadSock::Status ReadSock::getline(struct s_sock &sock, std::string &line) {
    int fd = sock.fd;
    if (fd < 0) {
        return INVALID_FD;
    }

    if (sock.perm & PERM_READ) {
        ReadSock::Status status = readSocket(fd);

        if (status == ReadSock::RECV_END) {
            return status; // ???
        }
        if (status == ReadSock::RECV_DONE) {
            sock.perm |= ~PERM_READ;
        }
    }

    size_t pos = _rems[fd].find("\r\n");
    if (pos == std::string::npos) {
        return LINE_NOT_FOUND;
    }

    line = _rems[fd].substr(0, pos);
    _rems[fd].erase(0, pos + 2);

    return LINE_FOUND;
}