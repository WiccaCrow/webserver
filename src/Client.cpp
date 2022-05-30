#include "Client.hpp"

namespace HTTP {

ReadSock Client::_reader;

Client::Client(struct pollfd &pfd, ServerBlock *servBlock)
    : _pfd(pfd)
    , _req(servBlock)
    , _servBlock(servBlock) {
}

Client::~Client() {
}

Client::Client(const Client &client)
    : _pfd(client._pfd)
    , _req(client._servBlock)
    , _servBlock(client._servBlock) {
    *this = client;
}

Client &
Client::operator=(const Client &client) {
    if (this != &client) {
        _res.setFormed(false);
        _req = client._req;
        _res = client._res;
        _servBlock = client._servBlock;
        // Not think it is correct
        _pfd = client._pfd;
    }
    return *this;
}

int
Client::getFd(void) {
    return _pfd.fd;
}

void
Client::changeFd(int fd) {
    _pfd.fd = fd;
}

bool
Client::responseFormed() {
    return _res.isFormed();
}

void
Client::receive(void) {
    std::string line;

    struct s_sock s = { _pfd.fd, ReadSock::PERM_READ };

    ReadSock::Status stat;
    while (true) {
        line = "";
        if (!(_req.getFlags() & PARSED_HEADERS) || !(_req.getFlags() & PARSED_SL)) {
            stat = _reader.getline(s, line);
        } else {
            stat = _reader.getline_for_chunked(s, line, _req);
        }
        Log.debug(line);
        switch (stat) {
            case ReadSock::RECV_END:
                disconnect();
            case ReadSock::INVALID_FD:
                return;
            case ReadSock::LINE_NOT_FOUND:
                return;

            case ReadSock::RECV_END_NB: {
                // Need to parse maybe
                // because the data could remain from the previous requests
                return;
            }

            case ReadSock::LINE_FOUND: {
                _req.parseLine(line);
                if (_req.getStatus() != HTTP::CONTINUE) {
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

void
Client::reply(void) {
    if (_pfd.fd == -1) {
        return;
    }
    std::cout << "     test 2 reply response" << std::endl;

    // std::cout << "res URI: " << _req.getPath() << std::endl;
    // определить размер данных, которые надо отправить
    // sendByte (по аналогии с recvServ) или sendSize

    // std::cout << << std::endl;

    int _req_getStatus = _req.getStatus();

    if (_req.getStatus() == HTTP::PROCESSING) {
        _req_getStatus = HTTP::OK;
    }
    std::cout << "test 1 reply response " << _req_getStatus << std::endl;
    if (_req_getStatus >= HTTP::BAD_REQUEST) {
        _res.findErr(_req_getStatus);
    } else if (_req_getStatus == 200) {
        std::cout << "test 4 reply response " << _req.getMethod() << std::endl;
        if (_req.getMethod() == "HEAD")
            _res.HEADmethod(_req);
        if (_req.getMethod() == "GET")
            _res.GETmethod(_req);
        if (_req.getMethod() == "POST")
            _res.POSTmethod(_req);
        if (_req.getMethod() == "DELETE")
            _res.DELETEmethod(_req);
    }
    long sentBytes = 0;
    // std::cout << "test 5 reply response" << std::endl;
    do {
        _res.SetLeftToSend(sentBytes);
        sentBytes += send(_pfd.fd, _res.GetLeftToSend(), _res.GetLeftToSendSize(), 0);
        if (sentBytes < 0) {
            // std::cout << "Disconnect 3" << std::endl;
            disconnect();
        }
    } while (static_cast<size_t>(sentBytes) < _res.GetResSize());
    _res.clear();
    _req.clear();

    if (_req_getStatus == HTTP::BAD_REQUEST || _req_getStatus == HTTP::REQUEST_TIMEOUT || _req_getStatus == HTTP::PAYLOAD_TOO_LARGE) {
        disconnect();
    }
    // std::cout << "test 6 reply response" << std::endl;
    // _res.resetResponse();

    // если нет каких-то полей с указанием окончания отправки ответа,
    // клиент будет продолжать стоять в ожидании окончания ответа - POLLOUT

    // if (sentBytes == 0) {
    //     disconnect();
    // }
}

void
Client::disconnect(void) {
    if (_pfd.fd != -1) {
        Log.info(to_string(_pfd.fd) + ": client left");

        close(_pfd.fd);
        _pfd.fd      = -1;
        _pfd.events  = 0;
        _pfd.revents = 0;
    }
}
}
