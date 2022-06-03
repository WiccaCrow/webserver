#pragma once

#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <map>

#include "ReadSock.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Status.hpp"

namespace HTTP {

class Client {

private:

    int             _fd;
    int             _port;
    std::string     _ipAddr;

    HTTP::Request   _req;
    HTTP::Response  _res;

    ServerBlock    *_servBlock;
    static ReadSock _reader;

public:
    Client(void);
    Client(const Client &client);
    ~Client();

    Client &operator=(const Client &client);

    void setFd(int fd);
    void setServerBlock(ServerBlock *serverBlock);

    void setIpAddr(const std::string);
    void setPort(int port);
    
    void linkToRequest(void);
    const std::string &getIpAddr() const;
    int getPort() const;
    const std::string getHostname() const;

    void process(void);
    void clearData(void);
    void checkIfFailed(void);

    void receive(void);
    void reply(void);
    int  getFd(void);
    bool responseFormed(void);
};

}
