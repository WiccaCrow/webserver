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
    int _fd;
    struct sockaddr_in _socketData;

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
    void setSocketData(struct sockaddr_in data);
    void setServerBlock(ServerBlock *serverBlock);

    void process(void);
    void clearData(void);
    void checkIfFailed(void);

    void receive(void);
    void reply(void);
    int  getFd(void);
    bool responseFormed(void);
};

}
