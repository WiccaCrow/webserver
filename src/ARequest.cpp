#include "ARequest.hpp"
#include "Server.hpp"
#include "Client.hpp"

namespace HTTP {

ARequest::ARequest(void)
    : _major(0)
    , _minor(0) 
    , _isChunkSize(false)
    , _expBodySize(-1)
    , _realBodySize(0)
    , _chunkSize(0)
    , _parseFlags(PARSED_NONE)
    , _headSent(false)
    , _bodySent(false)
    , _formed(false)
    , _chunked(false)
    , _isProxy(false)
    , _isCGI(false)
    , _parted(false)
    , _status(OK)
    , _fileaddr(NULL)
    , _filefd(-1)
    , _offset(0) {}

ARequest::~ARequest(void) {
    if (_filefd != -1) {
        close(_filefd);
    }
    if (_fileaddr != NULL) {
        // Log.debug() << "munmap file" << Log.endl;
        munmap(_fileaddr, getRealBodySize());
    }
}

Client *
ARequest::getClient(void) {
    return _client;
}

void
ARequest::setClient(Client *client) {
    _client = client;
}

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

int
ARequest::getFileFd(void) const {
    return _filefd;
}

const std::string &
ARequest::getFilename(void) const {
    return _filename;
}

void
ARequest::setBody(const std::string &body) {
    _body = body;
    setRealBodySize(body.length());
}

void
ARequest::appendBody(const std::string &body) {
    if (_filefd != -1) {
        const char *data = body.c_str();
        for (size_t i = 0; i < body.length(); ) {
            int bytes = write(_filefd, &data[i], body.length() - i);
            if (bytes < 0) {
                Log.syserr() << "ARequest:: Cannot write to tmp file" << Log.endl;
                return ;
            }
            i += bytes;
        }
    } else {
        _body += body;
    }
    setRealBodySize(getRealBodySize() + body.length());
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
ARequest::parted(void) const {
    return _parted;
}

void
ARequest::parted(bool parted) {
    _parted = parted;
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

int64_t
ARequest::getExpBodySize(void) const {
    return _expBodySize;
}

void
ARequest::setExpBodySize(int64_t size) {
    _expBodySize = size;
}

int64_t
ARequest::getRealBodySize(void) const {
    return _realBodySize;
}

void
ARequest::setRealBodySize(int64_t size) {
    _realBodySize = size;
}

void
ARequest::setStatus(StatusCode status) {
    _status = status;
}

bool
ARequest::createTmpFile(void) {

    char tmpl[] = "./.tmp/wsfileXXXXXX";
    _filefd = mkstemp(tmpl);

    if (_filefd == -1) {
        return false;
    }
    _filename = tmpl;
    return true;
}

bool
ARequest::mapFile(void) {

    if (_filefd < 0) {
        Log.error() << "open failed" <<Log.endl;
        return false;
    }

    if (fstat(_filefd, &_filestat) < 0) {
        Log.error() << "fstat failed" <<Log.endl;
        return false;
    }

    setRealBodySize(_filestat.st_size);

    _fileaddr = (char *)mmap(NULL, getRealBodySize(), PROT_READ, MAP_SHARED, _filefd, 0);
    if (_fileaddr == NULL) {
        Log.error() << "mmap failed" <<Log.endl;
        return false;   
    }

    std::string res;
    res.append(_fileaddr, getRealBodySize());

    return true;
}

bool
ARequest::tunnelGuard(bool value) {
    if (getClient()->isTunnel() && g_server->settings.blind_proxy) {
        return false;
    }
    return value;
}

std::string
ARequest::makeChunk(void) {

    std::string res;

    const uint64_t chunk_size = g_server->settings.chunk_size;
    const uint64_t filesize = getRealBodySize();

    if (_offset >= filesize) {
        // could be trailer headers
        res = "0" CRLF CRLF;
        chunked(false);

    } else {
        uint64_t size = chunk_size;
        if (filesize - _offset < chunk_size) {
            size = filesize - _offset;
        }

        res.assign(_fileaddr + _offset, size);
        res = itohs(size) + CRLF + res + CRLF;

        _offset += size;
    }
    return res;
}

std::string
ARequest::makePart(void) {

    std::string res;

    const uint64_t chunk_size = g_server->settings.chunk_size;
    const uint64_t filesize = getRealBodySize();

    if (_offset < filesize) {
        uint64_t size = chunk_size;
        if (filesize - _offset < chunk_size) {
            size = filesize - _offset;
        }

        res.assign(_fileaddr + _offset, size);
        _offset += size;
    }

    if (_offset >= filesize) {
       parted(false);
    }
    return res;
}

StatusCode
ARequest::writeChunk(const std::string &chunk) {

    if (_chunkSize >= chunk.length()) {
        appendBody(chunk);
        _chunkSize -= chunk.length();

    } else {
        // Maybe should be fixed to handle chunks that ended only with \n
        if (chunk[_chunkSize] != '\r' || 
            chunk[_chunkSize + 1] != '\n') {
            Log.error() << "ARequest:: Invalid chunk length" << Log.endl;
            return BAD_REQUEST;
        }
        appendBody(chunk.substr(0, chunk.length() - 2));
        _chunkSize = 0;
        isChunkSize(true);
    }

    return CONTINUE;
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

StatusCode
ARequest::writeBody(const std::string &body) {

    Log.debug() << "Response::writeBody" << Log.endl;

    appendBody(body);
    if (getRealBodySize() == getExpBodySize()) {
        Log.debug() << "ARequest:: Body processed" << Log.endl;
        setFlag(PARSED_BODY);
        return PROCESSING;
    }

    return CONTINUE;
}

StatusCode
ARequest::writePart(const std::string &body) {

    Log.debug() << "ARequest::writePart (CGI-only)" << Log.endl;

    if (body.empty()) {
        setFlag(PARSED_BODY);
        return PROCESSING;
    }
    appendBody(body);
    return CONTINUE;
}

StatusCode
ARequest::parseBody(const std::string &line) {

    uint64_t size = getRealBodySize() + line.length();

    if (size > g_server->settings.max_reg_upload_size && _filefd == -1) {
        if (!createTmpFile()) {
            Log.syserr() << "ARequest:: Unable to create tmp file" << Log.endl; 
            return INTERNAL_SERVER_ERROR;
        }
        Log.debug() << "ARequest:: tmp file " << _filename << " created" << Log.endl; 
    }

    if (chunked()) {
        return (isChunkSize() ? writeChunkSize(line) : writeChunk(line));
    } else if (has(CONTENT_LENGTH)) {
        return writeBody(line);
    } else {
        return writePart(line);
    }
}


}