#include "Server.hpp"

const size_t reservedClients = 128;

Server::Server() {
    _nbServBlocks = 0;
    _pollfds.reserve(reservedClients);
}

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

void Server::fillServBlocksFds(void) {
    for (size_t i = 0; i < _nbServBlocks; ++i) {
        _pollfds.push_back((struct pollfd){
            _ServBlocks[i].getServFd(),
            POLLIN,
            0});
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

int Server::pollInHandler(size_t id) {
    _pollfds[id].revents = 0;
    if (id < _nbServBlocks) {
        acceptNewClient(id);
        return 1;
    } else {
        _clients[id - _nbServBlocks].receive();
        return 0;
    }
}

void Server::pollHupHandler(size_t id) {
    Log.error("POLLHUP occured on the " + to_string(_pollfds[id].fd) + "socket");
    if (id >= _nbServBlocks) {
        _clients[id - _nbServBlocks].disconnect();
    }
}

void Server::pollOutHandler(size_t id) {
    if (_clients[id - _nbServBlocks].responseFormed()) {
        // _clients[id - _nbServBlocks].changeResponseFlag(0);
        _clients[id - _nbServBlocks].reply();
    }
}

void Server::pollErrHandler(size_t id) {
    Log.error("POLLERR occured on the " + to_string(_pollfds[id].fd) + "socket");
    exit(1);
}

void Server::start(void) {
    fillServBlocksFds();
    while (1) {
        _pollResult = 0;
        pollServ();
        const size_t size = _pollfds.size();
        for (size_t id = 0; id < size; id++) {
            if (_pollfds[id].revents & POLLIN) {
                if (pollInHandler(id)) {
                    break;
                }
            } else if (_pollfds[id].revents & POLLHUP) {
                pollHupHandler(id);
            } else if (_pollfds[id].revents & POLLOUT) {
                pollOutHandler(id);
            } else if (_pollfds[id].revents & POLLERR) {
                pollErrHandler(id);
            }
        }
    }
}

void Server::handlePollError() {
    Log.error(std::string("POLL: ") + strerror(errno));
    switch (errno) {
        case EFAULT: {
            break;
        }
        case EINTR: {
            break;
        }
        case EINVAL: {
            struct rlimit rlim;
            getrlimit(RLIMIT_NOFILE, &rlim);
            Log.error("ndfs: " + to_string(_pollfds.size()));
            Log.error("limits (soft, hard): (" +
                      to_string(rlim.rlim_cur) + ", " +
                      to_string(rlim.rlim_max) + ") ");
            break;
        }
        case ENOMEM: {
            break;
        }

        default:
            break;
    }

    // close all fds
    exit(1);
}

void Server::pollServ(void) {
    while (_pollResult == 0) {
        _pollResult = poll(_pollfds.data(), _pollfds.size(), 1000000);
    }
    if (_pollResult < 0) {
        handlePollError();
    }
}

void Server::handleAcceptError() {
    switch (errno) {
        case EWOULDBLOCK: {
            // OK, as we use non-blocking sockets
            break;
        }

        case EBADF: {
        }

        case ECONNABORTED: {
        }

        case EFAULT: {
        }

        case EINTR: {
        }

        case EINVAL: {
        }

        case EMFILE: {
        }

        case ENFILE: {
        }

        case ENOMEM: {
        }

        case ENOTSOCK: {
        }

        case EOPNOTSUPP: {
        }

        case EPROTO: {
        }

        case EPERM: {
            Log.error(std::string("ACCEPT: ") + strerror(errno));
        }

        default:
            break;
    }
}

static int fdNotTaken(struct pollfd pfd) {
    return pfd.fd == -1;
}

void Server::acceptNewClient(size_t id) {
    struct sockaddr_in addr;
    socklen_t          len = sizeof(addr);

    int fd = accept(_pollfds[id].fd, (struct sockaddr *)&addr, &len);

    if (fd < 0) {
        handleAcceptError();
        return;
    }

    if (fd > -1) {
        fcntl(fd, F_SETFL, O_NONBLOCK);

        std::vector<struct pollfd>::iterator it;
        it = std::find_if(_pollfds.begin(), _pollfds.end(), fdNotTaken);

        Log.info(to_string(fd) + ": client accepted");

        if (it != _pollfds.end()) {
            it->fd = fd;
            it->events = POLLIN | POLLOUT;
        } else {
            _pollfds.push_back((struct pollfd){fd, POLLIN | POLLOUT, 0});
            _clients.push_back(Client(_pollfds.back()));
        }
    }
}
