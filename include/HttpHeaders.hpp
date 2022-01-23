#pragma once

#include <map>
#include <string>

namespace HTTP {

struct HeaderHandler {
    void AcceptEncoding(const std::string &str);
    void AcceptLanguage(const std::string &str);
    void Authorization(const std::string &str);
    void CacheControl(const std::string &str);
    void Conection(const std::string &str);
    void Data(const std::string &str);
    void From(const std::string &str);
    void Host(const std::string &str);
    void IfMatch(const std::string &str);
    void IfModifiedSince(const std::string &str);
    void IfNoneMatch(const std::string &str);
    void IfRange(const std::string &str);
    void IfUnmodifiedSince(const std::string &str);
    void ProxyAuthorization(const std::string &str);
    void Range(const std::string &str);
    void Referer(const std::string &str);
    void TE(const std::string &str);
    void UserAgent(const std::string &str);
    void ContentLength(const std::string &str);
    void SetCookie(const std::string &str);
    void ContentType(const std::string &str);
};

struct Header {
    typedef void (HeaderHandler::*method)(const std::string &str);

    std::string line;

    size_t valStart;
    size_t keyLen;
    size_t valLen;

    method handler;
    uint32 hash; // ?

    const char *getKey() {
        return line.data();
    }

    const char *getVal() {
        return &line.data()[valStart];
    }
};

}; // namespace HTTP