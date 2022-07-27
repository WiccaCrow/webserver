#include "ARequest.hpp"

namespace HTTP {

ARequest::ARequest(void)
    : _major(0)
    , _minor(0) 
    , _isChunkSize(false)
    , _bodySize(0)
    , _chunkSize(0)
    , _parseFlags(PARSED_NONE)
    , _headSent(false)
    , _bodySent(false)
    , _formed(false)
    , _chunked(false)
    , _isProxy(false)
    , _isCGI(false)
    , _status(OK) {}

ARequest::~ARequest(void) {}

// ARequest::ARequest(const ARequest &other) {
//     *this = other;
// }

// ARequest &
// ARequest::operator=(const ARequest &other) {
//     if (this != &other) {
//         _minor        = other._minor;
//         _major        = other._major;
//         _protocol     = other._protocol;
//         _body         = other._body;
//         _head         = other._head;
//         _bodySize     = other._bodySize;
//         _isChunkSize  = other._isChunkSize;
//         _parseFlags   = other._parseFlags;
//         _chunkSize    = other._chunkSize;
//         _sent         = other._sent;
//         _formed       = other._formed;
//         _chunked      = other._chunked;
//         _status       = other._status;
//     }
//     return *this;
// }

const std::string &
ARequest::getBody(void) const {
    return _body;
}

const std::string &
ARequest::getHead(void) const {
    return _head;
}

const std::string &
ARequest::getProtocol(void) const {
    return _protocol;
}

int
ARequest::getMajor(void) const {
    return _major;
}

int
ARequest::getMinor(void) const {
    return _minor;
}

void
ARequest::setBody(const std::string &body) {
    _body = body;
}

void
ARequest::setHead(const std::string &head) {
    _head = head;
}

std::size_t
ARequest::getFlags(void) const {
    return _parseFlags;
}

StatusCode
ARequest::getStatus() const {
    return _status;
}

bool
ARequest::formed(void) const {
    return _formed;
}

void
ARequest::formed(bool formed) {
    _formed = formed;
}

bool
ARequest::sent(void) const {
    return headSent() && bodySent();
}

void
ARequest::sent(bool sent) {
    headSent(sent);
    bodySent(sent);
}

bool ARequest::headSent(void) const {
    return _headSent;
}

void ARequest::headSent(bool sent) {
    _headSent = sent;
}

bool ARequest::bodySent(void) const {
    return _bodySent;
}

void ARequest::bodySent(bool sent) {
    _bodySent = sent;
}

void
ARequest::setProtocol(const std::string &protocol) {
    _protocol = protocol;
}

void
ARequest::setMajor(int major) {
    _major = major;
}

void
ARequest::setMinor(int minor) {
    _minor = minor;
}

void
ARequest::setFlag(std::size_t flag) {
    _parseFlags |= flag;
}

void
ARequest::chunked(bool flag) {
    _chunked = flag;
}

bool
ARequest::chunked(void) const {
    return _chunked;
}

bool
ARequest::flagSet(std::size_t flag) const {
    return _parseFlags & flag;
}

// for chunked
bool
ARequest::isChunkSize(void) const {
    return _isChunkSize;
}

void
ARequest::isChunkSize(bool flag) {
    _isChunkSize = flag;
}

bool
ARequest::isProxy(void) {
    return _isProxy;
}

void
ARequest::isProxy(bool isProxy) {
    _isProxy = isProxy;
}

bool
ARequest::isCGI(void) {
    return _isCGI;
}

void
ARequest::isCGI(bool isCGI) {
    _isCGI = isCGI;
}

std::size_t
ARequest::getBodySize(void) const {
    return _bodySize;
}

void
ARequest::setBodySize(std::size_t size) {
    _bodySize = size;
}

void
ARequest::setStatus(StatusCode status) {
    _status = status;
}

StatusCode
ARequest::writeChunk(const std::string &chunk) {

    if (_chunkSize >= chunk.length()) {
        _body += chunk;
        _chunkSize -= chunk.length();

    } else {
        if (chunk[_chunkSize] != '\r' || 
            chunk[_chunkSize + 1] != '\n') {
            Log.error() << "ARequest:: Invalid chunk length" << Log.endl;
            return BAD_REQUEST;
        }
        _body += chunk.substr(0, chunk.length() - 2);
        _chunkSize = 0;
        isChunkSize(true);
    }

    return CONTINUE;
}

StatusCode
ARequest::parseChunk(const std::string &line) {    
    return (isChunkSize() ? writeChunkSize(line) : writeChunk(line));
}

StatusCode
ARequest::writeChunkSize(const std::string &line) {

    if (line.empty()) {
        Log.error() << "ARequest:: Chunk size is empty" << Log.endl;
        return BAD_REQUEST;
    }

    char *end = NULL;
    _chunkSize = strtoul(line.c_str(), &end, 16);

    if (!end || end[0] != '\r' || end[1] != '\n') {
        Log.error() << "ARequest:: Chunks size is invalid: " << line << Log.endl;
        return BAD_REQUEST;

    } else if (_chunkSize == ULONG_MAX) {
        Log.error() << "ARequest:: Chunk size is ULONG_MAX: " << line << Log.endl;
        return BAD_REQUEST;

    } else if (_chunkSize == 0 && line[0] != '0') {
        Log.error() << "ARequest:: Chunk size parsing failed: " << line << Log.endl;
        return BAD_REQUEST;

    } else if (_chunkSize == 0) {
        chunked(false);
        setFlag(PARSED_BODY);
        return PROCESSING;
    }

    isChunkSize(false);
    return CONTINUE;
}

}