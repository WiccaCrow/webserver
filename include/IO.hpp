#pragma once

#include <string>
#include <fcntl.h>
#include <cstddef>
#include <unistd.h>
#include <arpa/inet.h>

#include "Logger.hpp"
#include "Globals.hpp"

class IO {
    int         _fdr;
    int         _fdw;

    int         _af;
    std::string _addr;
    std::size_t _port;

    std::string _rem;

    const char *_data;
    std::size_t _dataSize;
    std::size_t _dataPos;

public:
    IO(void);
    ~IO(void);

    // void closeRdFd(void);
    // void closeWrFd(void);

    void rdFd(int);
    void wrFd(int);
    void setPort(std::size_t);
    void setDataPos(std::size_t);
    void setDataSize(std::size_t);
    void setData(const std::string &);
    void setAddr(const std::string &);

    int rdFd(void) const;
    int wrFd(void) const;
    std::size_t getPort(void) const;
    std::size_t getDataPos(void) const;
    std::size_t getDataSize(void) const;
    const char *getData(void) const;
    const std::string &getAddr(void) const;

    int read(void);
    int write(void);
    int nonblock(void);
    int getline(std::string &, std::size_t);

    int pipe(void);

    int socket(int = AF_INET);
    int connect(const sockaddr *, socklen_t);
    int listen(const std::string &addr, std::size_t port);

    void clear(void);

    void reset(void);
};