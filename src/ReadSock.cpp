#include "ReadSock.hpp"

static const size_t MAX_PACKET_SIZE = 65536;

ReadSock::Status
ReadSock::readSocket(int fd) {
    char buf[MAX_PACKET_SIZE + 1] = { 0 };

    int recvBytes = recv(fd, buf, MAX_PACKET_SIZE, 0);

    if (recvBytes < 0) {
        return RECV_NONBLOCK;

    } else if (recvBytes == 0) {
        _rems.erase(fd);
        return RECV_CLOSED;

    } else {
        buf[recvBytes + 1] = '\0';
        _rems[fd] += buf;
        return RECV_DONE;
    }
}

ReadSock::Status
ReadSock::getline(int fd, std::string &line) {

    if (readSocket(fd) == RECV_CLOSED) {
        return RECV_CLOSED;
    }

    size_t pos = _rems[fd].find("\r\n");
    if (pos == std::string::npos) {
        return LINE_NOT_FOUND;
    }

    line = _rems[fd].substr(0, pos + 2);
    _rems[fd].erase(0, pos + 2);

    return LINE_FOUND;
}
