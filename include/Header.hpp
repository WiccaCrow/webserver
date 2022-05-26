#pragma once

#include <map>
#include <string>
#include "HeadersCodes.hpp"
#include "StatusCodes.hpp"

namespace HTTP {

// struct HeaderHandler {};

class Header {
    public:
    typedef void (Header::*method)(void);

    std::string line;

    size_t valStart;
    size_t keyLen;
    size_t valLen;

    HeaderCode hash;

    const char *getKey() {
        return line.data();
    }

    const char *getVal() const {
        return &(line.data()[valStart]);
    }

    StatusCode AcceptEncoding(void);
    StatusCode AcceptLanguage(void);
    StatusCode Authorization(void);
    StatusCode CacheControl(void);
    StatusCode Conection(void);
    StatusCode Data(void);
    StatusCode From(void);
    StatusCode Host(void);
    StatusCode IfMatch(void);
    StatusCode IfModifiedSince(void);
    StatusCode IfNoneMatch(void);
    StatusCode IfRange(void);
    StatusCode IfUnmodifiedSince(void);
    StatusCode ProxyAuthorization(void);
    StatusCode Range(void);
    StatusCode Referer(void);
    StatusCode TE(void);
    StatusCode UserAgent(void);
    StatusCode ContentLength(void);
    StatusCode SetCookie(void);
    StatusCode ContentType(void);
};

} // namespace HTTP
