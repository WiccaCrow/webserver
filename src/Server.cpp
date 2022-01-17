#include "Server.hpp"

// COUNT_SERVERS - количество серверов
#define COUNT_SERVERS 1

//typedef struct pollfd pollfd;

enum ServerError {
    PARSE_ERR = -4,  //не использ
    CONNECT_ERR = -7 //не использ
};

/******************************************************************************/
/* Constructors */

//      init

Server::Server() {
    /**** заполнить переменные ****/
    _nbServBlocks = 0;
    assignPollFds();
};

//      copy
Server::Server(const Server &obj) {
    operator=(obj);
}

/******************************************************************************/
/* Destructors */
Server::~Server() {
    const size_t size = _pollfds.size();
    for (size_t i = _nbServBlocks; i < size; i++)
        close(_pollfds[i].fd);
}

/******************************************************************************/
/* Operators */

//      =
Server &Server::operator=(const Server &obj) {
    if (this != &obj) {
        _ServBlocks = obj._ServBlocks;
        _pollResult = obj._pollResult;
        _pollfds = obj._pollfds;
    }
    return (*this);
}

/******************************************************************************/
/* Private functions */

void Server::assignPollFds(void) {
    // _pollfds.reserve()
    _pollfds.assign(128, (struct pollfd){
                             -1,
                             POLLIN | POLLOUT,
                             0});
    _pollResult = 0;
}

void Server::fillServBlocksFds(void) {
    for (int i = 0; i < _nbServBlocks; ++i) {
        _pollfds[i].fd = _ServBlocks[i].getServFd();
        _pollfds[i].events = POLLIN;
        _pollfds[i].revents = 0;
    }
}

/******************************************************************************/
/* Public functions */

/* Set atributs */

void Server::resetPollEvents(void) {
    if (_pollResult) {
        const size_t size = _pollfds.size();
        for (size_t i = 0; i < size; i++)
            _pollfds[i].revents = 0;
        _pollResult = 0;
    }
}

/* Get and show atributs */

/* other methods */

void Server::addServerBlocks(ServerBlock &servBlock) {
    _ServBlocks.push_back(servBlock);
    _ServBlocks.back().start();
    _nbServBlocks++;
}

void Server::addServerBlocks(const std::string &ipaddr, const uint16_t port) {
    _ServBlocks.push_back(ServerBlock(ipaddr, port));
    _ServBlocks.back().start();
    _nbServBlocks++;
}

void Server::start(void) {
    fillServBlocksFds();
    while (1) {
        resetPollEvents();
        pollServ();
        const size_t size = _pollfds.size();
        for (size_t id = 0; id < size; id++) {
            if (id < COUNT_SERVERS && _pollfds[id].revents & POLLIN) {
                acceptNewClient(id);
            } else if (_pollfds[id].revents & POLLHUP) {
                std::cerr << "Cannot read from " << _pollfds[id].fd << " fd" << std::endl;
            } else if (_pollfds[id].revents & POLLOUT) {
                sendServ(id);
            } else if (_pollfds[id].revents & POLLIN) {
                recvServ(id);
            } else if (_pollfds[id].revents & POLLERR) {
                std::cerr << "Poll internal error" << std::endl;
                // close all fds
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

        // выдать ошибку
    }
}

void Server::recvServ(size_t i) {
    std::string line;
    Request     req;

    struct s_sock s = {_pollfds[i].fd, ReadSock::PERM_READ}; // temporal
    _pollfds[i].revents = 0;
    while (true) {
        line = "";
        ReadSock::Status stat = ReadSock::getline(s, line);

        switch (stat) {
            case ReadSock::RECV_END:
            case ReadSock::RECV_FAIL: {
                disconnectClient(i);
                return;
            }

            case ReadSock::INVALID_FD: {
                return;
            }

            case ReadSock::LINE_NOT_FOUND: {
                return;
            }

            case ReadSock::LINE_FOUND: {
                // req.parseLine(line);
                break;
            }
        }
    }

    //header (struct maybe): key, value
    //split header line by ':' and trim whitespaces of each part
    //then insert into hash-table or list or tree

    // Parse body (if exist)

    //Request:  method, path, protocol, headers, [body],
    //Reponse:  protocol, status(code), status(message)  headers, [body]

    // hashtable -> location ???

    // если от клиента пришел запрос, обработать
    // флажок revents сменится на POLLOUT для выдачи ответа recv
}
}

void Server::sendServ(size_t id) {
    if (_pollfds[id].fd == -1) {
        return;
    }

    // определить размер данных, которые надо отправить
    // sendByte (по аналогии с recvServ) или sendSize
    char buf[PACKET_SIZE] = "message to send\n";
    int  sendSize = strlen(buf);
    if (sendSize > PACKET_SIZE)
        sendSize = PACKET_SIZE;
    // если нет каких-то полей с указанием окончания отправки ответа,
    // клиент будет продолжать стоять в ожидании окончания ответа - POLLOUT
    int send_byte = send(_pollfds[id].fd, buf, sendSize, 0);
    if (send_byte < 0) {
        disconnectClient(id);
        ; // error case
    }
    if (send_byte == 0) {
        disconnectClient(id);
    }
    if (send_byte > 0) {
        ; // выдача ответа send
    }
}

static int isFreeFD(struct pollfd pfd) {
    return pfd.fd == -1;
}

void Server::acceptNewClient(size_t id) {
    struct sockaddr_in cliaddr;
    socklen_t          addrlen = sizeof(cliaddr);

    int client_socket = accept(_ServBlocks[id].getServFd(), (struct sockaddr *)&cliaddr, &addrlen);
    if (client_socket > -1) {
        std::vector<struct pollfd>::iterator it = std::find_if(_pollfds.begin(), _pollfds.end(), isFreeFD);

        if (it != _pollfds.end()) {
            it->fd = client_socket;
            it->events = POLLIN | POLLOUT;
            it->revents = 0;
        } else {
            _pollfds.push_back((struct pollfd){
                client_socket,
                POLLIN | POLLOUT,
                0});
        }
    }
}

void Server::disconnectClient(size_t id) {
    if (_pollfds[id].fd != -1) {
        close(_pollfds[id].fd);
        _pollfds[id].fd = -1;
        _pollfds[id].events = 0;
        _pollfds[id].revents = 0;
    }
}