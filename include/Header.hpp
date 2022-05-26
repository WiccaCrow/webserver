#pragma once

#include <map>
#include <string>
#include "HeadersCodes.hpp"
#include "StatusCodes.hpp"

namespace HTTP {

class Header {
    public:
    typedef StatusCode (Header::*Handler) (void);

    std::string line;

    size_t valStart;
    size_t keyLen;
    size_t valLen;
    Handler handler;
    HeaderCode hash;

    const char *getKey() {
        return line.data();
    }

    const char *getVal() const {
        return &(line.data()[valStart]);
    }

    StatusCode A_IM(void);
    StatusCode Accept(void);
    StatusCode AcceptCharset(void);
    StatusCode AcceptEncoding(void);
    StatusCode AcceptLanguage(void);
    StatusCode AcceptDateTime(void);
    StatusCode AccessControlRequestMethod(void);
    StatusCode AccessControlRequestHeaders(void);
    StatusCode Authorization(void);
    StatusCode CacheControl(void);
    StatusCode Connection(void);
    StatusCode Cookie(void);
    StatusCode Date(void);
    StatusCode Expect(void);
    StatusCode Forwarded(void);
    StatusCode From(void);
    StatusCode Host(void);
    StatusCode IfMatch(void);
    StatusCode IfModifiedSince(void);
    StatusCode IfNoneMatch(void);
    StatusCode IfRange(void);
    StatusCode IfUnmodifiedSince(void);
    StatusCode MaxForwards(void);
    StatusCode Origin(void);
    StatusCode Pragma(void);
    StatusCode ProxyAuthorization(void);
    StatusCode Range(void);
    StatusCode Referer(void);
    StatusCode TransferEncoding(void);
    StatusCode TE(void);
    StatusCode UserAgent(void);
    StatusCode ContentLength(void);
    // StatusCode SetCookie(void);
    StatusCode ContentType(void);
    StatusCode Upgrade(void);
    StatusCode Via(void);
    StatusCode Warning(void);
    StatusCode Dnt(void);
    StatusCode XRequestedWith(void);
    StatusCode XCsrfToken(void);
    StatusCode SecFetchDest(void);
    StatusCode SecFetchMode(void);
    StatusCode SecFetchSite(void);
    StatusCode SecFetchUser(void);
    StatusCode UpgradeInsecureRequests(void);
    StatusCode SecChUa(void);
    StatusCode SecGpc(void);
    StatusCode SecChUaMobile(void);
    StatusCode SecChUaPlatform(void);

    StatusCode NotSupported(void);
    
    };

} // namespace HTTP
