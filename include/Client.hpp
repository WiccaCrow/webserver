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
    int             _clientPort;
    int             _serverPort;
    std::string     _ipAddr;
    bool            _requestFormed;
    bool            _responseFormed;

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
    int getFd(void) const;

    void setPort(int port);
    int getPort(void) const;

    void setServerPort(int port);
    int getServerPort(void) const;

    void setIpAddr(const std::string);
    const std::string &getIpAddr(void) const;

    void linkToRequest(void);
    const std::string getHostname(void) const;

    bool isRequestFormed() const;
    void setRequestFormed(bool);

    bool isResponseFormed() const;
    void setResponseFormed(bool);

    void receive(void);
    void process(void);
    void reply(void);
    void checkIfFailed(void);
    void clearData(void);
};

}
