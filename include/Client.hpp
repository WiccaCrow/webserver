#pragma once

#include <string>
#include <unistd.h>
#include <deque>

#include "Request.hpp"
#include "Response.hpp"
#include "Logger.hpp"
#include "Status.hpp"
#include "Globals.hpp"
#include "AClient.hpp"

namespace HTTP {

class Client : public AClient { 

private:
    size_t      _clientPort;
    std::string _clientIpAddr;
    bool _headSent;
    bool _bodySent;

    Request *_req;
    std::deque<HTTP::Response *> _responses;

public:
    Client(void);
    Client(const Client &client);
    ~Client();

    Client &operator=(const Client &client);

    const std::string getHostname(void) const;

    Request *    getRequest(void);
    void         setRequest(Request *);
    Response *   getResponse(void);

    virtual void pollin(void);
    virtual void pollout(void);
    virtual void pollhup(void);
    virtual void pollerr(void);

    virtual void receive(void);
    virtual void reply(void);

    void setPort(size_t port);
    size_t  getPort(void) const;

    void               setIpAddr(const std::string &);
    const std::string &getIpAddr(void) const;

    void checkIfFailed(void);
    void addRequest(void);
    void addResponse(Response *);
    void removeResponse(void);

    bool validSocket(void);
    bool requestReady(void);
    bool replyReady(void);
    bool replyDone(void);
    void replyDone(bool);

    // HTTP::ServerBlock *matchServerBlock(const std::string &host, size_t port) const;
};

}
