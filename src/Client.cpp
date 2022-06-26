#include "Client.hpp"
#include "Server.hpp"

namespace HTTP {

Client::Client()
    : _fdIn(-1)
    , _fdOut(-1)
    , _clientPort(0)
    , _serverPort(0)
    , _shouldBeClosed(false)
    , _reqPoolReady(true)
    , _replyDone(false)
    , _id(0) {
}

Client::~Client() {
    for (size_t i = 0; i < _requests.size(); i++) {
        if (_requests[i] != NULL) {
            delete _requests[i];
        }
    }
    
    for (size_t i = 0; i < _responses.size(); i++) {
        if (_responses[i] != NULL) {
            delete _responses[i];
        }
    }
}

Client::Client(const Client &client) {
    *this = client;
}

Client &
Client::operator=(const Client &other) {
    if (this != &other) {
        _requests       = other._requests;
        _responses      = other._responses;
        _reqPoolReady   = other._reqPoolReady;
        _fdIn           = other._fdIn;
        _fdOut          = other._fdOut;
        _clientIpAddr   = other._clientIpAddr;
        _serverIpAddr   = other._serverIpAddr;
        _clientPort     = other._clientPort;
        _serverPort     = other._serverPort;
        _shouldBeClosed = other._shouldBeClosed;
        _replyDone      = other._replyDone;
        _proxy          = other._proxy;
        _id             = other._id;
    }
    return *this;
}

void
Client::initResponseMethodsHeaders(void) {}

void
Client::setFdIn(int fd) {
    _fdIn = fd;
}

int
Client::getFdIn(void) const {
    return _fdIn;
}

void
Client::setFdOut(int fd) {
    _fdOut = fd;
}

int
Client::getFdOut(void) const {
    return _fdOut;
}

void
Client::setId(size_t id) {
    _id = id;
}

size_t
Client::getId(void) const {
    return _id;
}

void
Client::setIpAddr(const std::string &ipaddr) {
    _clientIpAddr = ipaddr;
}

void
Client::setServerIpAddr(const std::string &ipaddr) {
    _serverIpAddr = ipaddr;
}

const std::string &
Client::getServerIpAddr(void) const {
    return _serverIpAddr;
}


void
Client::setPort(size_t port) {
    _clientPort = port;
}

size_t
Client::getPort(void) const {
    return _clientPort;
}

void
Client::setServerPort(size_t port) {
    _serverPort = port;
}

size_t
Client::getServerPort(void) const {
    return _serverPort;
}

const std::string &
Client::getIpAddr(void) const {
    return _clientIpAddr;
}

const std::string
Client::getHostname() const {
    return (_clientPort != 0 ? _clientIpAddr + ":" + sztos(_clientPort) : _clientIpAddr);
}

Request *
Client::getRequest() {
    return _requests.back();
}

Response *
Client::getResponse() {
    return _responses.back();
}

void
Client::shouldBeClosed(bool flag) {
    _shouldBeClosed = flag;
}

bool
Client::shouldBeClosed(void) const {
    return _shouldBeClosed;
}

bool
Client::isProxy(void) {
    return (_proxy.on());
}

bool
Client::readForProxy() {
    bool isProxy(_fdIn != _fdOut);
    // std::cout << "is Proxy " << isProxy << std::endl;
    if (isProxy) {
        _rem = "";
        if (!readSocket()) {
            setFdIn(-1);
        } else {
            getRequest()->setBody(_rem);
            getRequest()->isFormed(true);
        }
    }
    return isProxy;
}

void
Client::addRequest(void) {

    Request *req = new Request(this);
    if (req == NULL) {
        Log.syserr() << "Cannot allocate request" << std::endl;
        setFdIn(-1);
    }

    Log.debug() << "----------------------" << std::endl;
    
    _requests.push_back(req);
    requestPoolReady(false);
}

void
Client::addResponse(Response *res) {
    // Response *res = new Response(getRequest());
    // if (res == NULL) {
    //     Log.syserr() << "Cannot allocate response" << std::endl;
    //     setFdIn(-1);
    // }
    _responses.push_back(res);
    // requestPoolReady(true);
}

void
Client::removeTopRequest(void) {
    _requests.pop_front();
}

void
Client::removeTopResponse(void) {
    delete _responses.front();
    _responses.pop_front();
}

bool
Client::requestPoolReady(void) {
    return _reqPoolReady;
}

void
Client::requestPoolReady(bool flag) {
    _reqPoolReady = flag;
}

Request *
Client::getTopRequest(void) {
    return _requests.front();
}

Response *
Client::getTopResponse(void) {
    return _responses.front();
}

Proxy *
Client::getProxy(void) {
    return &_proxy;
}

StatusCode
Client::getProxyStatus(void) {
    return _proxy.getStatus();
}

void
Client::setProxyUri(URI *uri) {
    _proxy.setUri(uri);
}

void
Client::setProxyFdOut(int fd) {
    _proxy.setFdOut(fd);
}

void
Client::setProxyidOtherSide(size_t id) {
    _proxy.idOtherSide(id);
}

bool
Client::validSocket(void) {
    return _fdIn != -1;
}

bool
Client::requestReady(void) {
    return validSocket() && _requests.size() && getTopRequest()->isFormed();
}

bool
Client::replyReady(void) {
    return validSocket() && _responses.size() && getTopResponse()->isFormed();
}

bool
Client::replyDone(void) {
    return _replyDone;
}

void
Client::replyDone(bool flag) {
    _replyDone = flag;
}


void
Client::checkIfFailed(void) {

    static const size_t size = 4;
    static const StatusCode failedStatuses[size] = {
        BAD_REQUEST,
        REQUEST_TIMEOUT,
        INTERNAL_SERVER_ERROR,
        PAYLOAD_TOO_LARGE
    };

    for (size_t i = 0; i < size; i++) {
        if (getTopResponse()->getStatus() == failedStatuses[i]) {
            setFdIn(-1);
            break ;
        }
    }
}

void
Client::process(void) {

    Log.debug() << "Client::process [" << _fdIn << "]" << std::endl;
    if (_fdIn != _fdOut) { //  первый ответ клиенту отправляется на прежний  fd 
    std::cout << "_fdIn = " << _fdIn << ", _fdOut = " << _fdOut << std::endl;
        getTopResponse()->makeProxyResponse(getTopRequest()->getBody());
    } else {
        getTopResponse()->handle();
    }
}

void
Client::reply(void) {

    signal(SIGPIPE, SIG_IGN);

    size_t all = 0;
    for (size_t total = getTopResponse()->getResponseLength(); total;
                total = getTopResponse()->getResponseLength()) {
        const char *rsp = getTopResponse()->getResponse().c_str();

        size_t sent = 0;
        do {
            long n = send(_fdOut, rsp + sent, total - sent, 0);
            Log.debug() << "fdOut " << _fdOut << std::endl;
            if (n > 0) {
                sent += n;
            }
            else if (n == 0) {
                Log.debug() << "Client::send 0 returned (disc)" << std::endl;
                if (_fdIn != _fdOut) {
                    setFdOut(-1);
                }
                break ;
            }
            else {
                // if (_fdIn != _fdOut) {
                //     setFdOut(getFdIn());
                //     getTopResponse()->setErrorResponse(BAD_GATEWAY);
                //     getTopResponse()->makeFullResponse();
                //     continue ;
                // }
                Log.debug() << "Client::send -1 returned" << std::endl;
                return ;
            }
        } while (sent < total);

        all += sent;
        Log.debug() << "Client::reply [" << _fdIn << "] (" << sent << "/" << total << " bytes sent)" << std::endl;

        if (_fdIn != _fdOut) {
            break ;
        }

        getTopResponse()->makeChunk();
    }

    Log.debug() << "Client::Total sent: " << all << std::endl;
    
    _replyDone = true;
    
    if (isProxy() && _fdOut != -1) {
        _fdOut = _proxy.getFdOut();
    }

    if (shouldBeClosed()) {
        setFdIn(-1);
    }
}


static const size_t MAX_PACKET_SIZE = 65536;

int
Client::readSocket(void) {
    char buf[MAX_PACKET_SIZE + 1] = { 0 };

    int recvBytes = recv(_fdIn, buf, MAX_PACKET_SIZE, 0);

    if (recvBytes == 0) {
        _rem.erase();

    } else if (recvBytes > 0) {
        buf[recvBytes] = '\0';
        _rem += buf;
    }
    return recvBytes;
}

Client::Status
Client::getline(std::string &line) {

    if (!readSocket()) {
        return SOCK_CLOSED;
    }

    size_t pos = 0;
    const size_t bodySize = getTopRequest()->getBodySize();
    if (!bodySize) {
        pos = _rem.find("\r\n");
        if (pos == std::string::npos) {
            return LINE_NOT_FOUND;
        }
        pos += 2;
    } else {
        if (_rem.length() < bodySize) {
            return LINE_NOT_FOUND;
        }
        pos = bodySize;
    }

    line = _rem.substr(0, pos);
    _rem.erase(0, pos);

    return LINE_FOUND;
}

void
Client::proxyRun(void) {
    setProxyUri(&getRequest()->getUriRef());
    size_t id = _proxy.run();
    // id ==0 error
    getRequest()->setStatus(_proxy.getStatus());

    if (getRequest()->getStatus() == OK) {
        g_server->getClient(id)->setProxyFdOut(getFdIn());
        g_server->getClient(id)->_fdOut = _fdIn;
        g_server->getClient(id)->setProxyidOtherSide(getId());
        Log.debug() << "Proxy. Other side:"
                  << "\t\tclient fdIn  = " << g_server->getClient(id)->getFdIn()  << std::endl
                  << "\t\tclient fdOut = " << g_server->getClient(id)->getFdOut() << std::endl;
    }
}

void
Client::receive(void) {

    if (_fdIn < 0) {
        return;
    }

    Log.debug() << "Client::receive [" << _fdIn << "]" << std::endl;

    if (readForProxy()) {
        return ;
    }
   std::cout << "Client::receive" << std::endl;

    while (!getRequest()->isFormed()) {
        std::string line;

        switch (getline(line)) {
            case LINE_FOUND: {
                // std::cout << "         test line: "<< line << std::endl;
                getRequest()->parseLine(line);
                if (getRequest()->getMethod() == "CONNECT" && !getProxy()->on()) {
                    proxyRun();
                }
                break ;
            }
            case SOCK_CLOSED: {
                setFdIn(-1);
                return ;
            }
            case LINE_NOT_FOUND: {
                return ;
            }
        }
    }
}

HTTP::ServerBlock *
Client::matchServerBlock(const std::string &host) const {
    typedef std::list<HTTP::ServerBlock>::iterator iter_l;
    typedef std::list<HTTP::ServerBlock> bslist;

    bool searchByName = !isValidIpv4(host) ? true : false;

    bslist &blocks = g_server->getServerBlocks(_serverPort);
    iter_l found = blocks.end();
    for (iter_l block = blocks.begin(); block != blocks.end(); ++block) {
        if (block->hasAddr(getServerIpAddr())) {
            if (found == blocks.end()) {
                found = block;
                if (!searchByName) {
                    break ;
                }
            }
            if (searchByName && block->hasName(host)) {
                found = block;
                break ;
            }
        }
    }
    Log.debug() << "Client:: servBlock " << found->getBlockName() << " for " << host << ":" << _serverPort << std::endl;
    return &(*found);
}

}
