#pragma once

#include <map>
#include <poll.h>
#include <sys/socket.h>
#include <string>
#include <unistd.h>
#include <queue>

#include "Request.hpp"
#include "Response.hpp"
#include "Logger.hpp"
#include "Status.hpp"
#include "Globals.hpp"

namespace HTTP {

class Client {

private:
    int         _fd;
    int         _clientPort;
    int         _serverPort;
    std::string _ipAddr;

    std::queue<HTTP::Request> _requests;
    std::queue<HTTP::Response> _responses;

    HTTP::Request  _req;
    HTTP::Response _res;

    bool        _shouldBeClosed;

    std::string _rem;

    int readSocket(void);
    

public:
    enum Status {
        LINE_NOT_FOUND = -1,
        SOCK_CLOSED    = 0,
        LINE_FOUND     = 1
    };

public:
    Client(void);
    Client(const Client &client);
    ~Client();

    Client &operator=(const Client &client);

    void initResponseMethodsHeaders(void);

    Status getline(std::string &line);

    void setFd(int fd);
    int  getFd(void) const;

    void setPort(int port);
    int  getPort(void) const;

    void setServerPort(int port);
    int  getServerPort(void) const;

    void               setIpAddr(const std::string);
    const std::string &getIpAddr(void) const;

    void              linkRequest(void);
    const std::string getHostname(void) const;

    const Request &    getRequest() const;
    const Response &   getResponse() const;

    void receive(void);
    void process(void);
    void reply(void);
    void checkIfFailed(void);
    void clearData(void);

    bool            shouldBeClosed(void) const;
    void            shouldBeClosed(bool);

    HTTP::ServerBlock *matchServerBlock(const std::string &host) const;
};

}
