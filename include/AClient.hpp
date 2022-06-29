#pragma once

#include <sys/socket.h>
#include <string>
#include <unistd.h>
#include <deque>

// #include "Request.hpp"
// #include "Response.hpp"
#include "Logger.hpp"
// #include "Status.hpp"
// #include "Globals.hpp"

namespace HTTP {

class AClient {

    int _fdr;
    int _fdw;
    size_t      _serverPort;
    std::string _serverIpAddr;

    std::string _rem;

    bool        _shouldBeClosed;

    const char *_data;
    size_t      _dataSize;
    size_t      _dataPos;

public:
    int  readSocket(void);

    void setData(const char *);
    void setDataSize(size_t);
    void setDataPos(size_t);
    bool sendData(void);

    const char * getData(void) const;
    size_t getDataSize(void) const;
    size_t getDataPos(void) const;

    int getline(std::string &, size_t);

    AClient(void);
    AClient(const AClient &);
    AClient &operator=(const AClient &other);
    virtual ~AClient(void);

    int getReadFd(void) const;
    int getWriteFd(void) const;
    void setReadFd(int fd);
    void setWriteFd(int fd);

    void setServerPort(size_t port);
    size_t getServerPort(void) const;

    void               setServerIpAddr(const std::string &);
    const std::string &getServerIpAddr(void) const;

    virtual void pollin(void) = 0;
    virtual void pollout(void) = 0;
    virtual void pollhup(void) = 0;
    virtual void pollerr(void) = 0;

    virtual void receive(void) = 0;
    virtual void reply(void) = 0;
    
    bool shouldBeClosed(void) const;
    void shouldBeClosed(bool);
};

}