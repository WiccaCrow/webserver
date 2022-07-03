#pragma once

#include <unistd.h>

#include <deque>
#include <string>

#include "Globals.hpp"
#include "IO.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Status.hpp"

namespace HTTP {

class Client {
    private:
    IO *_clientIO;
    IO *_serverIO;
    IO *_targetIO;

    bool _headSent;
    bool _bodySent;
    bool _shouldBeClosed;

    std::size_t _nbRequests;
    std::size_t _maxRequests;
    std::time_t _clientTimeout;
    std::time_t _targetTimeout;
    std::time_t _maxTimeout;

    std::list<Request *>  _requests;
    std::list<Response *> _responses;

    public:
    Client(void);
    ~Client(void);

    bool replyDone(void);
    void replyDone(bool);

    bool shouldBeClosed(void) const;
    void shouldBeClosed(bool);

    const std::string getHostname(void);

    time_t getClientTimeout(void) const;
    time_t getTargetTimeout(void) const;

    void setClientTimeout(time_t);
    void setTargetTimeout(time_t);

    IO  *getClientIO(void);
    IO  *getServerIO(void);
    IO  *getTargetIO(void);
    void setClientIO(IO *);
    void setServerIO(IO *);
    void setTargetIO(IO *);

    void checkIfFailed(void);
    void addRequest(void);
    void addResponse(void);
    void removeRequest(void);
    void removeResponse(void);

    void pollin(int fd);
    void pollout(int fd);
    void pollhup(int fd);
    void pollerr(int fd);

    void receive(Request *);
    void receive(Response *);
    void reply(Request *);
    void reply(Response *);

    ServerBlock *matchServerBlock(const std::string &host);
};

} // namespace HTTP
