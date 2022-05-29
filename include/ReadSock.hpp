#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <sys/socket.h>

#include "Logger.hpp"
#include "Request.hpp"

class ReadSock {

public:
    enum Perm {
        PERM_NONE  = 0,
        PERM_READ  = 1,
        PERM_WRITE = 2
    };

    enum Status {
        RECV_DONE      = 2,
        LINE_FOUND     = 1,
        RECV_END       = 0,
        INVALID_FD     = -1,
        RECV_END_NB    = -2,
        LINE_NOT_FOUND = -3
    };

private:
    std::map<int, std::string> _rems;

    Status readSocket(int fd);
    Status readSocket_chunked(int fd);
    int    readSocket_for_chunked(struct s_sock &sock, int fd);

public:
    Status getline(struct s_sock &sock, std::string &line);
    Status getline_for_chunked(struct s_sock &sock, std::string &line,
        HTTP::Request &req);
};

// Where should I put this ... ?
struct s_sock {
    int16_t  fd;
    u_int8_t perm;
};
