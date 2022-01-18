#pragma once

#include <sys/socket.h>

#include <map>
#include <stdexcept>
#include <string>

class ReadSock {
    public:
    enum Perm {
        PERM_NONE = 0,
        PERM_READ = 1,
        PERM_WRITE = 2,
    };

    enum Status {
        RECV_DONE = 2,
        LINE_FOUND = 1,
        RECV_END = 0,
        INVALID_FD = -1,
        RECV_FAIL = -2,
        LINE_NOT_FOUND = -3
    };

    private:
    std::map<int, std::string> _rems;

    Status readSocket(int fd);

    public:
    Status getline(struct s_sock &sock, std::string &line);
};

// Where should I put this ... ?
struct s_sock {
    int16_t  fd;
    u_int8_t perm;
};