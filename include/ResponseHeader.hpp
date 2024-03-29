#pragma once

#include <map>
#include <set>
#include <string>
#include <stdint.h>
#include <algorithm>

#include "Utils.hpp"
#include "Header.hpp"
#include "Status.hpp"
#include "Globals.hpp"
#include "HeadersCodes.hpp"

namespace HTTP {

class Response;

class ResponseHeader : public Header {

public:
    typedef void (ResponseHeader::*Handler)(Response &res);
    Handler     method;

    bool isValid(void);

    ResponseHeader();
    ResponseHeader(uint32_t hash);
    ResponseHeader(uint32_t hash, const std::string &value);

    void handle(Response &res);

    void AcceptPatch(Response &res);
    void AcceptRanges(Response &res);
    void Age(Response &res);
    void Allow(Response &res);
    void AltSvc(Response &res);
    void CacheControl(Response &res);
    void Connection(Response &res);
    void ContentDisposition(Response &res);
    void ContentEncoding(Response &res);
    void ContentLanguage(Response &res);
    void ContentLength(Response &res);
    void ContentLocation(Response &res);
    void ContentRange(Response &res);
    void ContentType(Response &res);
    void Date(Response &res);
    void DeltaBase(Response &res);
    void Expires(Response &res);
    void ETag(Response &res);
    void IM(Response &res);
    void KeepAlive(Response &res);
    void Host(Response &res);
    void LastModified(Response &res);
    void Link(Response &res);
    void Location(Response &res);
    void Pragma(Response &res);
    void ProxyAuthenticate(Response &res);
    void PublicKeyPins(Response &res);
    void RetryAfter(Response &res);
    void Server(Response &res);
    void SetCookie(Response &res);
    void StrictTransportSecurity(Response &res);
    void Trailer(Response &res);
    void TransferEncoding(Response &res);
    void Tk(Response &res);
    void Upgrade(Response &res);
    void Vary(Response &res);
    void Via(Response &res);
    void Warning(Response &res);
    void WWWAuthenticate(Response &res);

    // Non-standard
    void ContentSecurityPolicy(Response &res);
    void Refresh(Response &res);
    void XPoweredBy(Response &res);
    void XRequestID(Response &res);
    void XUACompatible(Response &res);
    void XXSSProtection(Response &res);

    // Cors
    void AccessControlAllowOrigin(Response &res);
    void AccessControlAllowCredentials(Response &res);
    void AccessControlExposeHeaders(Response &res);
    void AccessControlMaxAge(Response &res);
    void AccessControlAllowMethods(Response &res);
    void AccessControlAllowHeaders(Response &res);

    void NotSupported(Response &res);
};

extern const std::map<uint32_t, ResponseHeader::Handler> validResHeaders;

} // namespace HTTP
