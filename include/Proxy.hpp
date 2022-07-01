#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "Globals.hpp"
#include "Logger.hpp"
#include "URI.hpp"
#include "Status.hpp"
#include "Response.hpp"

namespace HTTP {

class Proxy {

private:
    URI         *_uri;
    std::string  _host;
    std::string  _port;
    bool         _on;
    StatusCode   _status;
    int          _fdOut;
    std::size_t       _idOtherSide;
    Response    *_res;

public:
    Proxy(void);
    Proxy(URI *);
    ~Proxy(void);

    Proxy(const Proxy &other);
    Proxy &operator=(const Proxy &other);

    void clear(void);

    void   setUri(URI *uri);
    std::size_t run(void);

    addrinfo *setConnection(addrinfo *p);

    void       setStatus(StatusCode code);
    StatusCode getStatus(void);

    void  setFdOut(int fd);
    int   getFdOut(void);

    bool on(void);
    void on(bool onOff);

    void   idOtherSide(std::size_t id);
    std::size_t idOtherSide(void);

};

}
