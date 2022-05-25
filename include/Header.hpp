#pragma once

#include <map>
#include <string>
#include "Types.hpp"

namespace HTTP {

// struct HeaderHandler {};

struct Header {
    typedef void (Header::*method)(void);

    std::string line;

    size_t valStart;
    size_t keyLen;
    size_t valLen;

    method handler;
    uint32 hash; // ?

    const char *getKey() {
        return line.data();
    }

    const char *getVal() const {
        return &(line.data()[valStart]);
    }

    void AcceptEncoding(void);
    void AcceptLanguage(void);
    void Authorization(void);
    void CacheControl(void);
    void Conection(void);
    void Data(void);
    void From(void);

    void Host(void) {

    }
    
    void IfMatch(void);
    void IfModifiedSince(void);
    void IfNoneMatch(void);
    void IfRange(void);
    void IfUnmodifiedSince(void);
    void ProxyAuthorization(void);
    void Range(void);
    void Referer(void);
    void TE(void);
    void UserAgent(void);
    void ContentLength(void);
    void SetCookie(void);
    void ContentType(void);
};

}; // namespace HTTP