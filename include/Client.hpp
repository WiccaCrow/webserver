#pragma once

#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <map>

#include "ReadSock.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "StatusCodes.hpp"

namespace HTTP {

class Client {

private:
    struct pollfd &_pfd;
    HTTP::Request  _req;
    HTTP::Response _res;

    ServerBlock    *_servBlock;
    static ReadSock _reader;

public:
    Client(struct pollfd &pfd, ServerBlock *servBlock);
    Client(const Client &client);
    ~Client();

    Client &operator=(const Client &client);

    void disconnect(void);
    void receive(void);
    void reply(void);
    int  getFd(void);
    void changeFd(int fd);
    bool responseFormed(void);
};

}
