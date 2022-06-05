#include "ResponseHeader.hpp"
#include "Request.hpp"
#include "Response.hpp"

namespace HTTP {

StatusCode
ResponseHeader::AcceptPatch(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::AcceptRanges(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Age(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Allow(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::AltSvc(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::CacheControl(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Connection(Response &res) {
    (void)res;
    if (res.getRequest()->getHeaderValue(CONNECTION) == "close") {
        value = "close";
    } else {
        value = "keep-alive";
    }
    return CONTINUE;
}

StatusCode
ResponseHeader::ContentDisposition(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::ContentEncoding(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::ContentLanguage(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::ContentLength(Response &res) {
    (void)res;
    value = to_string(res.getBody().length());
    return CONTINUE;
}

StatusCode
ResponseHeader::ContentLocation(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::ContentRange(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::ContentType(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Date(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::DeltaBase(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Expires(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::ETag(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::IM(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Host(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::KeepAlive(Response &res) {
    (void)res;
    value = "timeout=55, max=1000";
    return CONTINUE;
}

StatusCode
ResponseHeader::LastModified(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Link(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Location(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Pragma(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::ProxyAuthenticate(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::PublicKeyPins(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::RetryAfter(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Server(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::SetCookie(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::StrictTransportSecurity(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Trailer(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::TransferEncoding(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Tk(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Upgrade(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Vary(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Via(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Warning(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::WWWAuthenticate(Response &res) {
    (void)res;
    return CONTINUE;
}

// Non-standard
StatusCode
ResponseHeader::ContentSecurityPolicy(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::Refresh(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::XPoweredBy(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::XRequestID(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::XUACompatible(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::XXSSProtection(Response &res) {
    (void)res;
    return CONTINUE;
}

// CORS
StatusCode
ResponseHeader::AccessControlAllowOrigin(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::AccessControlAllowCredentials(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::AccessControlExposeHeaders(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::AccessControlMaxAge(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::AccessControlAllowMethods(Response &res) {
    (void)res;
    return CONTINUE;
}

StatusCode
ResponseHeader::AccessControlAllowHeaders(Response &res) {
    (void)res;
    return CONTINUE;
}

}