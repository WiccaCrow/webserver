#include "Client.hpp"

ReadSock Client::_reader;

Client::Client(struct pollfd& pfd) : _pfd(pfd), _responseFormed(0) {
}

Client::~Client() {
}

Client::Client(const Client& client) : _pfd(client._pfd) {
    *this = client;
}

Client& Client::operator=(const Client& client) {
    if (this != &client) {
        _responseFormed = client._responseFormed;
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

void Client::changeResponseFlag(bool f) {
    _responseFormed = f;
}

bool Client::responseFormed() {
    return _responseFormed;
}

void Client::receive(void) {
    std::string line;

    _responseFormed = 1;
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
                //_req.parseLine(line);
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

    // CONTINUE = 100,
    // SWITCHING_PROTOCOLS = 101,
    // PROCESSING = 102,
    // EARLY_HINTS = 103,
    // OK = 200,
    // CREATED = 201,
    // ACCEPTED = 202,
    // NON_AUTHORITATIVE_INFORMATION = 203,
    // NO_CONTENT = 204,
    // RESET_CONTENT = 205,
    // PARTIAL_CONTENT = 206,
    // MULTI_STATUS = 207,
    // ALREADY_REPORTED = 208,
    // IM_USED = 226,
    // MULTIPLE_CHOICES = 300,
    // MOVED_PERMANENTLY = 301,
    // FOUND = 302,
    // SEE_OTHER = 303,
    // NOT_MODIFIED = 304,
    // USE_PROXY = 305,
    // TEMPORARY_REDIRECT = 307,
    // PERMANENT_REDIRECT = 308,
    // UNAUTHORIZED = 401,
    // PAYMENT_REQUIRED = 402,
    // FORBIDDEN = 403,

    // NOT_ACCEPTABLE = 406,
    // PROXY_AUTHENTICATION_REQUIRED = 407,
    // CONFLICT = 409,
    // GONE = 410,
    // LENGTH_REQUIRED = 411,
    // PRECONDITION_FAILED = 412,
    // PAYLOAD_TOO_LARGE = 413,
    // URI_TOO_LONG = 414,
    // UNSUPPORTED_MEDIA_TYPE = 415,
    // RANGE_NOT_SATISFIABLE = 416,
    // EXPECTATION_FAILED = 417,
    // IM_A_TEAPOT = 418,
    // AUTHENTICATION_TIMEOUT = 419,
    // MISDIRECTED_REQUEST = 421,
    // UNPROCESSABLE_ENTITY = 422,
    // LOCKED = 423,
    // FAILED_DEPENDENCY = 424,
    // TOO_EARLY = 425,
    // UPGRADE_REQUIRED = 426,
    // PRECONDITION_REQUIRED = 428,
    // TOO_MANY_REQUESTS = 429,
    // REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    // RETRY_WITH = 449,
    // UNAVAILABLE_FOR_LEGAL_REASONS = 451,
    // CLIENT_CLOSED_REQUEST = 499,
    // INTERNAL_SERVER_ERROR = 500,
    // NOT_IMPLEMENTED = 501,
    // BAD_GATEWAY = 502,
    // SERVICE_UNAVAILABLE = 503,
    // GATEWAY_TIMEOUT = 504,
    // HTTP_VERSION_NOT_SUPPORTED = 505,
    // VARIANT_ALSO_NEGOTIATES = 506,
    // INSUFFICIENT_STORAGE = 507,
    // LOOP_DETECTED = 508,
    // BANDWIDTH_LIMIT_EXCEEDED = 509,
    // NOT_EXTENDED = 510,
    // NETWORK_AUTHENTICATION_REQUIRED = 511,
    // UNKNOWN_ERROR = 520,
    // WEB_SERVER_IS_DOWN = 521,
    // CONNECTION_TIMED_OUT = 522,
    // ORIGIN_IS_UNREACHABLE = 523,
    // A_TIMEOUT_OCCURRED = 524,
    // SSL_HANDSHAKE_FAILED = 525,
    // INVALID_SSL_CERTIFICATE = 526

    // const char*  response = _res.getData();
    const size_t responseLength = strlen(response);
    size_t       sentBytes = send(_pfd.fd, response, responseLength, 0);

    _responseFormed = 0;
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
