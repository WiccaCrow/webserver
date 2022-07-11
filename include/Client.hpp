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
    IO *_gatewayIO;

    bool _shouldBeClosed;
    bool _shouldBeRemoved;
    bool _isTunnel;

    std::size_t _nbRequests;
    std::size_t _maxRequests;
    std::time_t _clientTimeout;
    std::time_t _gatewayTimeout;
    // std::time_t _maxTimeout;

    std::string _domain;

    std::list<Request *>  _requests;
    std::list<Response *> _responses;

    int _id;

public:
    std::size_t links;

    Client(void);
    ~Client(void);

    void checkTimeout(void);

    void tryReplyResponse(int fd);
    void tryReplyRequest(int fd);
    void tryReceiveResponse(int fd);
    void tryReceiveRequest(int fd);

    bool shouldBeClosed(void) const;
    void shouldBeClosed(bool);

    bool shouldBeRemoved(void) const;
    void shouldBeRemoved(bool);

    bool isTunnel(void) const;
    void isTunnel(bool);

    const std::string getHostname(void);

    int getId(void) const;
    void setId(int);

    time_t getClientTimeout(void) const;
    time_t getGatewayTimeout(void) const;

    void setClientTimeout(time_t);
    void setGatewayTimeout(time_t);

    IO  *getClientIO(void);
    IO  *getServerIO(void);
    IO  *getGatewayIO(void);
    void setClientIO(IO *);
    void setServerIO(IO *);
    void setGatewayIO(IO *);

    const std::string &getDomainName(void) const;
    void setDomainName(const std::string &);

    void checkIfFailed(void);
    void addRequest(void);
    void addResponse(void);
    void removeRequest(void);
    void removeResponse(void);

    void receive(Request *);
    void receive(Response *);
    void reply(Request *);
    void reply(Response *);

    ServerBlock *matchServerBlock(const std::string &host);
};

} // namespace HTTP
