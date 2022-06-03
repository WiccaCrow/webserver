#include "Client.hpp"

namespace HTTP {

ReadSock Client::_reader;

Client::Client() {
}

Client::~Client() {
}

Client::Client(const Client &client) {
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
        _fd = client._fd;
    }
    return *this;
}

int
Client::getFd(void) {
    return _fd;
}

// Why would I need to do that? I know now :D
void
Client::setFd(int fd) {
    _fd = fd;
}


bool
Client::responseFormed() {
    return _res.isFormed();
}

void
Client::setSocketData(struct sockaddr_in data) {
    _socketData = data;
}

void
Client::setServerBlock(ServerBlock *serverBlock) {
    _servBlock = serverBlock;
    _req.setServerBlock(_servBlock);
}

// void
// Client::setPollFdPtr(struct pollfd *ptr) {
//     _pfd = ptr;
// }


void
Client::receive(void) {

    std::string line;

    struct s_sock s = { _fd, ReadSock::PERM_READ };

    Log.debug("Client::receive -> fd:" + to_string(_fd));

    ReadSock::Status stat;
    while (true) {
        line = "";
        if (!(_req.getFlags() & PARSED_HEADERS) || !(_req.getFlags() & PARSED_SL)) {
            stat = _reader.getline(s, line);
        } else {
            stat = _reader.getline_for_chunked(s, line, _req);
        }
        switch (stat) {
            case ReadSock::RECV_END:
                Log.debug("Client::recv_env " + to_string(_fd));
                setFd(-1);
                // need to erase from map of clients in server
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
Client::checkIfFailed(void) {

    if (_req.getStatus() == HTTP::BAD_REQUEST || 
        _req.getStatus() == HTTP::REQUEST_TIMEOUT || 
        _req.getStatus() == HTTP::INTERNAL_SERVER_ERROR ||
        _req.getStatus() == HTTP::PAYLOAD_TOO_LARGE) {
        setFd(-1);
    }
}

void
Client::clearData(void) {
    _res.clear();
    _req.clear();
}

void
Client::process(void) {
    if (_fd == -1) {
        return;
    }

    Log.debug("Client::process -> fd:"  + to_string(_fd));

    HTTP::StatusCode status = _req.getStatus();

    if (status == HTTP::PROCESSING) {
        status = HTTP::OK;
    }

    if (status >= HTTP::BAD_REQUEST) {
        _res.setErrorResponse(status);
    } else if (status == HTTP::OK) {
        if (_req.getMethod() == "HEAD")
            _res.HEADmethod(_req);
        else if (_req.getMethod() == "GET")
            _res.GETmethod(_req);
        else if (_req.getMethod() == "POST")
            _res.POSTmethod(_req);
        else if (_req.getMethod() == "DELETE")
            _res.DELETEmethod(_req);
    }
}

void
Client::reply(void) {
    if (_fd == -1) {
        return;
    }

    Log.debug("Client::reply -> fd:"  + to_string(_fd));

    long sentBytes = 0;
    do {
        _res.setLeftToSend(sentBytes);
        sentBytes += send(_fd, _res.getLeftToSend(), _res.getLeftToSendSize(), 0);
        if (sentBytes < 0) {
            _req.setStatus(INTERNAL_SERVER_ERROR);
            break;
        }
    } while (static_cast<size_t>(sentBytes) < _res.getResSize());
    
   
    // std::cout << "test 6 reply response" << std::endl;
    // _res.resetResponse();

    // если нет каких-то полей с указанием окончания отправки ответа,
    // клиент будет продолжать стоять в ожидании окончания ответа - POLLOUT

    // if (sentBytes == 0) {
    //     disconnect();
    // }
}

// void
// Client::disconnect(void) {
//     _fd = -1;
    // if (_fd != -1) {

    //     // Maybe socket should be closed on the same level as accept, i.e. in server class
    //     // close(_fd);
    //     _fd = -1;
        
    //     // _pfd->events  = 0;
    //     // _pfd->revents = 0;
    // }
// }

}
