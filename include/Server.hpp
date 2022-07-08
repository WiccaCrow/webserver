#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/resource.h>
#include <unistd.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <queue>
#include <vector>

#include "Client.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerBlock.hpp"
#include "Utils.hpp"
#include "Worker.hpp"

class Server {
    public:
    typedef std::list<HTTP::ServerBlock> ServersList;
    typedef ServersList::iterator        iter_sl;

    typedef std::map<std::size_t, ServersList> ServersMap;
    typedef ServersMap::iterator               iter_sm;

    typedef std::vector<struct pollfd> PollFdVec;
    typedef PollFdVec::iterator        iter_pfd;

    typedef std::vector<IO *>    SocketsVec;
    typedef SocketsVec::iterator iter_sv;

    typedef std::map<int, HTTP::Client *> ClientsMap;
    typedef ClientsMap::iterator          iter_cm;

    private:
    ServersMap _servers;
    SocketsVec _sockets;
    ClientsMap _clients;
    PollFdVec  _pollfds;

    bool   _working;
    Worker _workers[WORKERS];

    pthread_mutex_t _m_new_resp;

    pthread_mutex_t _m_new_pfds;
    pthread_mutex_t _m_new_clnt;

    pthread_mutex_t _m_del_pfds;
    pthread_mutex_t _m_del_clnt;

    std::list<HTTP::Response *> _q_newResponses;

    std::queue<std::pair<int, HTTP::Client *> >  _q_newClients;
    std::queue<int>                              _q_newPfds;

    std::set<HTTP::Client *>  _q_delClients;
    std::set<int>             _q_delPfds;

    public:
    bool isDaemon;

    Server(void);
    Server(const Server &);
    ~Server(void);
    Server &operator=(const Server &);

    ServersList &operator[](std::size_t port);
    ServersMap  &getServerBlocks(void);

    bool working(void);
    void start(void);
    void finish(void);

    // void disconnect(int fd);
    void addToNewFdsQ(int);
    void addToNewClientQ(int fd, HTTP::Client *);
    void addToDelFdsQ(int);
    void addToDelClientQ(HTTP::Client *);

    void addToRespQ(HTTP::Response *);
    HTTP::Response *rmFromRespQ(void);

    private:
    void connect(std::size_t servid, int servfd);

    void daemonMode(void);
    int  poll(void);
    void process(void);
    void checkTimeout(void);

    void createSockets(void);
    void startWorkers(void);
    void stopWorkers(void);

    int addListenSocket(const std::string &addr, std::size_t port);

    void emptyNewFdsQ(void);
    void emptyDelFdsQ(void);
    void emptyNewClientQ(void);
    void emptyDelClientQ(void);
};
