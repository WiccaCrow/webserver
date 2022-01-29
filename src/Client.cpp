#include "Client.hpp"

ReadSock Client::_reader;

Client::Client(struct pollfd& pfd) : _pfd(pfd) {
}

Client::~Client() {
}

Client::Client(const Client& client) : _pfd(client._pfd) {
    *this = client;
}

Client& Client::operator=(const Client& client) {
    if (this != &client) {
        _res.setFormed(false);
        _req = client._req;
        _res = client._res;
        // Not think it is correct
        _pfd = client._pfd;
    }
    return *this;
}

int Client::getFd(void) {
    return _pfd.fd;
}

void Client::changeFd(int fd) {
    _pfd.fd = fd;
}

// void Client::changeResponseFlag(bool f) {
//     _responseFormed = f;
// }

bool Client::responseFormed() {
    return _res.isFormed();
}

void Client::receive(void) {
    std::string line;

    // _responseFormed = 1;
    struct s_sock s = {_pfd.fd, ReadSock::PERM_READ};
    while (true) {
        line = "";
        ReadSock::Status stat = _reader.getline(s, line);
        Log.debug(line);
        switch (stat) {
            case ReadSock::RECV_END:
                disconnect();
            case ReadSock::INVALID_FD:
                return;
            case ReadSock::LINE_NOT_FOUND:
                //_responseFormed = 1;
                return;

            case ReadSock::RECV_END_NB: {
                // Need to parse maybe
                // because the data could remain from the previous requests
                return;
            }

            case ReadSock::LINE_FOUND: {
                if (_req.parseLine(line) == HTTP::PROCESSING) {
                    _res.setFormed(true);
                    return;
                }
                break;
            }
            default: {
                return;
            }
        }
    }
}

void Client::reply(void) {
    if (_pfd.fd == -1) {
        return;
    }

    // определить размер данных, которые надо отправить
    // sendByte (по аналогии с recvServ) или sendSize

    // _req.getStatus() еще не написано, но уже обговорено.
    // это будет либо status в pubic у Request, либо геттер на него
    const char* response;
    switch (HTTP::OK) {
            // switch (_req.getStatus()) {
        case HTTP::OK:
            response = _res.getData();
            break;
        case HTTP::BAD_REQUEST:
            response = _res.ErrorCli400();
            break;
        case HTTP::NOT_FOUND:
            response = _res.ErrorCli404(_req.getProtocol());
            break;
        case HTTP::METHOD_NOT_ALLOWED:
            response = _res.ErrorCli405(_req.getProtocol());
            break;
        case HTTP::REQUEST_TIMEOUT:
            response = _res.ErrorCli408(_req.getProtocol());
            disconnect();
            break;
        default:
            // временная строка, чтобы был response
            response = _res.getData();
            break;
    }

    const size_t responseLength = strlen(response);
    size_t       sentBytes = send(_pfd.fd, response, responseLength, 0);
    _res.setFormed(false);

    // если нет каких-то полей с указанием окончания отправки ответа,
    // клиент будет продолжать стоять в ожидании окончания ответа - POLLOUT

    // Not sure if it should be here.
    // Most likely current function should return the value (or set some flag)
    // to the server class and it should disconnect the client

    if (sentBytes < 0) {
        disconnect();
        // Error case
    }
    if (sentBytes == 0) {
        disconnect();
    }
    if (sentBytes != responseLength) {
        // Not all bytes were sent
        // Chucked response or error
    }
}

void Client::disconnect(void) {
    if (_pfd.fd != -1) {
        Log.info(to_string(_pfd.fd) + ": client left");

        close(_pfd.fd);
        _pfd.fd = -1;
        _pfd.events = 0;
        _pfd.revents = 0;
    }
}
