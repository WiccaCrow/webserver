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

// ReadSock::Status ReadSock::readSocket_chunked(int fd) {
//     char buf[MAX_PACKET_SIZE + 1] = {0};
//
//     int recvBytes = recv(fd, buf, MAX_PACKET_SIZE, 0);
//
//     if (recvBytes < 0) {
//         //_rems.erase(fd); // Not needed with nonblocking sockets
//         return RECV_END_NB;
//
//     } else if (recvBytes == 0) {
//         _rems.erase(fd);
//         return RECV_END;
//
//     } else {
//         buf[recvBytes + 1] = '\0';
//         _rems[fd] += buf;
//         return RECV_DONE;
//     }
// }

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

    if (!readSocket_for_chunked(sock, fd)) {
        return ReadSock::RECV_END;
    }

    if (req.getChunked_isSizeChunk()) {
        size_t pos = _rems[fd].find("\r\n");
        if (pos == std::string::npos) {
            return LINE_NOT_FOUND;
        }
        line = _rems[fd].substr(0, pos);
        _rems[fd].erase(0, pos + 2);
        return LINE_FOUND;
    } else {
        long chunkSize = req.getChunked_Size();
        std::cout << chunkSize << "test getline chunked body 1 " << _rems[fd].length() << std::endl;
        size_t remsLength = _rems[fd].length();
        for (; remsLength <= chunkSize + 2;
             remsLength += _rems[fd].length()) {
            // std::cout << "test getline chunked body " << remsLength << std::endl;

            line = _rems[fd];
            _rems[fd].clear();
            if (!readSocket_for_chunked(sock, fd)) {
                return ReadSock::RECV_END;
            }
        }
        std::cout << "test getline chunked body 2 " << std::endl;
        if (_rems[fd][_rems[fd].length() - (remsLength - req.getChunked_Size())] == '\n') {
            std::cout << "test n" << std::endl;
        } else if (_rems[fd][_rems[fd].length() - (remsLength - req.getChunked_Size()) - 1] == '\r') {
            std::cout << "test r" << std::endl;
        }

        if (_rems[fd][chunkSize] != '\r' &&
            _rems[fd][chunkSize + 1] != '\n') {
            return LINE_NOT_FOUND;
        }
        line = _rems[fd].substr(0, chunkSize);
        req.setChunked_isSizeChunk(true);
        req.setChunked_Size(0);
    }

    return LINE_FOUND;
}

ReadSock::Status ReadSock::getline(struct s_sock &sock, std::string &line) {
    int fd = sock.fd;
    if (fd < 0) {
        return INVALID_FD;
    }

    if (sock.perm & PERM_READ) {
        Log.debug("Readsock:34, before readSocket");
        ReadSock::Status status = readSocket(fd);
        Log.debug("Readsock:36, after readSocket");

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

// Location /index.html {
//     rewrite ^/oldURL$ https://www.your_domain.com/newURL redirect;
// }
// if (rewrite.empty() == false) {
//     func();
//     return 301;
// }
