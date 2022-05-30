#pragma once

#include <map>
#include <string>
#include <stdint.h>

#include "StatusCodes.hpp"
#include "HeadersCodes.hpp"

namespace HTTP {

class Request;

class Header {

public:
    typedef StatusCode (Header::*Handler)(Request &req);
    
    std::string line;

    size_t     valStart;
    size_t     keyLen;
    size_t     valLen;
    Handler    method;
    HeaderCode hash;

    const char *getKey() {
        return line.data();
    }

    const char *getVal() const {
        return &(line.data()[valStart]);
    }

    StatusCode handle(Request &req);
    StatusCode A_IM(Request &req);
    StatusCode Accept(Request &req);
    StatusCode AcceptCharset(Request &req);
    StatusCode AcceptEncoding(Request &req);
    StatusCode AcceptLanguage(Request &req);
    StatusCode AcceptDateTime(Request &req);
    StatusCode AccessControlRequestMethod(Request &req);
    StatusCode AccessControlRequestHeaders(Request &req);
    StatusCode Authorization(Request &req);
    StatusCode CacheControl(Request &req);
    StatusCode Connection(Request &req);
    StatusCode Cookie(Request &req);
    StatusCode Date(Request &req);
    StatusCode Expect(Request &req);
    StatusCode Forwarded(Request &req);
    StatusCode From(Request &req);
    StatusCode Host(Request &req);
    StatusCode IfMatch(Request &req);
    StatusCode IfModifiedSince(Request &req);
    StatusCode IfNoneMatch(Request &req);
    StatusCode IfRange(Request &req);
    StatusCode IfUnmodifiedSince(Request &req);
    StatusCode MaxForwards(Request &req);
    StatusCode Origin(Request &req);
    StatusCode Pragma(Request &req);
    StatusCode ProxyAuthorization(Request &req);
    StatusCode Range(Request &req);
    StatusCode Referer(Request &req);
    StatusCode TransferEncoding(Request &req);
    StatusCode TE(Request &req);
    StatusCode UserAgent(Request &req);
    StatusCode ContentLength(Request &req);
    // StatusCode SetCookie(Request &req);
    StatusCode ContentType(Request &req);
    StatusCode Upgrade(Request &req);
    StatusCode Via(Request &req);
    StatusCode Warning(Request &req);
    StatusCode Dnt(Request &req);
    StatusCode XRequestedWith(Request &req);
    StatusCode XCsrfToken(Request &req);
    StatusCode SecFetchDest(Request &req);
    StatusCode SecFetchMode(Request &req);
    StatusCode SecFetchSite(Request &req);
    StatusCode SecFetchUser(Request &req);
    StatusCode UpgradeInsecureRequests(Request &req);
    StatusCode SecChUa(Request &req);
    StatusCode SecGpc(Request &req);
    StatusCode SecChUaMobile(Request &req);
    StatusCode SecChUaPlatform(Request &req);

    StatusCode NotSupported(Request &req);
};

extern std::map<uint32_t, Header::Handler> validHeaders;

} // namespace HTTP

