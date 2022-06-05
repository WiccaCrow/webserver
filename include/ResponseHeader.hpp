#pragma once

#include <algorithm>
#include <map>
#include <set>
#include <stdint.h>
#include <string>

#include "Globals.hpp"
#include "HeadersCodes.hpp"
#include "Status.hpp"

namespace HTTP {

class Response;

class ResponseHeader {

public:
    typedef StatusCode (ResponseHeader::*Handler)(Response &res);

    std::string key;
    std::string value;
    Handler     method;
    HeaderCode  hash;

    StatusCode handle(Response &res);

    StatusCode AcceptPatch(Response &res);
    StatusCode AcceptRanges(Response &res);
    StatusCode Age(Response &res);
    StatusCode Allow(Response &res);
    StatusCode AltSvc(Response &res);
    StatusCode CacheControl(Response &res);
    StatusCode Connection(Response &res);
    StatusCode ContentDisposition(Response &res);
    StatusCode ContentEncoding(Response &res);
    StatusCode ContentLanguage(Response &res);
    StatusCode ContentLength(Response &res);
    StatusCode ContentLocation(Response &res);
    StatusCode ContentRange(Response &res);
    StatusCode ContentType(Response &res);
    StatusCode Date(Response &res);
    StatusCode DeltaBase(Response &res);
    StatusCode Expires(Response &res);
    StatusCode ETag(Response &res);
    StatusCode IM(Response &res);
    StatusCode KeepAlive(Response &res);
    StatusCode Host(Response &res);
    StatusCode LastModified(Response &res);
    StatusCode Link(Response &res);
    StatusCode Location(Response &res);
    StatusCode Pragma(Response &res);
    StatusCode ProxyAuthenticate(Response &res);
    StatusCode PublicKeyPins(Response &res);
    StatusCode RetryAfter(Response &res);
    StatusCode Server(Response &res);
    StatusCode SetCookie(Response &res);
    StatusCode StrictTransportSecurity(Response &res);
    StatusCode Trailer(Response &res);
    StatusCode TransferEncoding(Response &res);
    StatusCode Tk(Response &res);
    StatusCode Upgrade(Response &res);
    StatusCode Vary(Response &res);
    StatusCode Via(Response &res);
    StatusCode Warning(Response &res);
    StatusCode WWWAuthenticate(Response &res);

    // Non-standard
    StatusCode ContentSecurityPolicy(Response &res);
    StatusCode Refresh(Response &res);
    StatusCode XPoweredBy(Response &res);
    StatusCode XRequestID(Response &res);
    StatusCode XUACompatible(Response &res);
    StatusCode XXSSProtection(Response &res);

    // Cors
    StatusCode AccessControlAllowOrigin(Response &res);
    StatusCode AccessControlAllowCredentials(Response &res);
    StatusCode AccessControlExposeHeaders(Response &res);
    StatusCode AccessControlMaxAge(Response &res);
    StatusCode AccessControlAllowMethods(Response &res);
    StatusCode AccessControlAllowHeaders(Response &res);

    StatusCode NotSupported(Response &res);
};

// extern const std::map<uint32_t, ResponseHeader::Handler> validHeaders;

} // namespace HTTP
