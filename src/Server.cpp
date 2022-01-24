#include "Server.hpp"

const size_t reservedClients = 128;

Server::Server() {
    _nbServBlocks = 0;
    assignPollFds();
};

Server::Server(const Server &obj) {
    operator=(obj);
}

Server::~Server() {
    const size_t size = _pollfds.size();
    for (size_t i = 0; i < size; i++) {
        if (_pollfds[i].fd != -1) {
            close(_pollfds[i].fd);
            _pollfds[i].fd = -1;
        }
    }
}

Server &Server::operator=(const Server &obj) {
    if (this != &obj) {
        _ServBlocks = obj._ServBlocks;
        _pollResult = obj._pollResult;
        _pollfds = obj._pollfds;
    }
    return (*this);
}

// Private functions
void Server::assignPollFds(void) {
    _pollfds.assign(reservedClients, (struct pollfd){-1, POLLIN | POLLOUT, 0});

    for (size_t i = _nbServBlocks; i < reservedClients; i++) {
        _clients.push_back(Client(_pollfds[i]));
    }

    _pollResult = 0;
}

void Server::fillServBlocksFds(void) {
    for (size_t i = 0; i < _nbServBlocks; ++i) {
        _pollfds[i].fd = _ServBlocks[i].getServFd();
        _pollfds[i].events = POLLIN;
        _pollfds[i].revents = 0;
    }
}

// Public functions

// Set attributes

// Get and show attributes

// Other methods

void Server::addServerBlocks(ServerBlock &servBlock) {
    _ServBlocks.push_back(servBlock);
    _ServBlocks.back().createListenSock();
    _nbServBlocks++;
}

void Server::addServerBlocks(const std::string &ipaddr, const uint16_t port) {
    _ServBlocks.push_back(ServerBlock(ipaddr, port));
    _ServBlocks.back().createListenSock();
    _nbServBlocks++;
}

void Server::start(void) {
    fillServBlocksFds();
    while (1) {
        pollServ();
        const size_t size = _pollfds.size();
        for (size_t id = 0; id < size; id++) {
            if (_pollfds[id].revents & POLLIN) {
                if (id < _nbServBlocks) {
                    acceptNewClient(id);
                    break;
                } else {
                    _pollfds[id].revents = 0;
                    _clients[id].receive();
                }
            } else if (_pollfds[id].revents & POLLHUP) {
                std::cerr << _pollfds[id].fd << ": POLLHUP" << std::endl;
                // Basically occurs when client closes his socket
                if (id >= _nbServBlocks) {
                    _clients[id].disconnect();
                }
            } else if (_pollfds[id].revents & POLLOUT) {
                if (_clients[id].responseFormed()) {
                    _clients[id].reply();
                }
            } else if (_pollfds[id].revents & POLLERR) {
                std::cerr << "POLLERR" << std::endl;
                // Close all fds ? throw exception
                exit(1);
            }
        }
    }
}

void Server::pollServ(void) {
    while (_pollResult == 0) {
        _pollResult = poll(_pollfds.data(), _pollfds.size(), 10000);
    }
    if (_pollResult < 0) {
        std::cerr << "Poll internal error" << std::endl;
        // close all fds
        exit(1);
    }
}

static int isFreeFD(struct pollfd pfd) {
    return pfd.fd == -1;
}

void Server::acceptNewClient(size_t id) {
    struct sockaddr_in cliaddr;
    socklen_t          addrlen = sizeof(cliaddr);

    _pollfds[id].revents = 0;
    int fd = accept(_ServBlocks[id].getServFd(), (struct sockaddr *)&cliaddr, &addrlen);
    // checkErrno(); некритические в основном
    if (fd > -1) {
        fcntl(fd, F_SETFL, O_NONBLOCK);
        std::vector<struct pollfd>::iterator it;
        it = std::find_if(_pollfds.begin(), _pollfds.end(), isFreeFD);
        std::cout << fd << ": client accepted" << std::endl;

        if (it != _pollfds.end()) {
            it->fd = fd;
            //it->revents = 0;
        } else {
            _pollfds.push_back((struct pollfd){fd, POLLIN | POLLOUT, 0});
            _clients.push_back(Client(_pollfds.back()));
        }
    }
}
