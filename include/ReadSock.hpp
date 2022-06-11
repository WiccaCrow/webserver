#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <sys/socket.h>

#include "Logger.hpp"
#include "Request.hpp"

class ReadSock {

public:

    enum Status {
        RECV_DONE      = 2,
        LINE_FOUND     = 1,
        RECV_CLOSED    = 0,
        INVALID_FD     = -1,
        RECV_NONBLOCK  = -2,
        LINE_NOT_FOUND = -3
    };

private:
    std::map<int, std::string> _rems;

    Status readSocket(int fd);

public:
    Status getline(int fd, std::string &line);

};
