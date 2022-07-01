#pragma once

#include <string>
#include <fcntl.h>
#include <cstddef>
#include <unistd.h>
#include <arpa/inet.h>

#include "Logger.hpp"
#include "Globals.hpp"

class Socket {
    int         _fd;

    int         _af;
    std::string _addr;
    std::size_t _port;

    std::string _rem;

    const char *_data;
    std::size_t _dataSize;
    std::size_t _dataPos;

public:
    Socket(void);
    ~Socket(void);

    void setFd(int);
    void setPort(std::size_t);
    void setDataPos(std::size_t);
    void setDataSize(std::size_t);
    void setData(const char *);
    void setAddr(const std::string &);

    int getFd(void) const;
    std::size_t getPort(void) const;
    std::size_t getDataPos(void) const;
    std::size_t getDataSize(void) const;
    const char *getData(void) const;
    const std::string &getAddr(void) const;

    int read(void);
    int write(void);
    int nonblock(void);
    int getline(std::string &, std::size_t);

    int create(int = AF_INET);
    int connect(const sockaddr *, socklen_t);
    int listen(const std::string &addr, std::size_t port);

    void clear(void);
};