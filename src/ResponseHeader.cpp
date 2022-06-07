#include "ResponseHeader.hpp"
#include "Request.hpp"
#include "Response.hpp"

namespace HTTP {

std::string
getDateTimeGMT() {
    // e.g. Date: Wed, 21 Oct 2015 07:28:00 GMT
    time_t rawtime;
    time(&rawtime);
    struct tm *info = gmtime(&rawtime);
    
    char buff[70];
    strftime(buff, sizeof(buff), "%a, %-e %b %Y %H:%M:%S GMT", info);
    
    return buff;
}

void
ResponseHeader::handleHeader(Response &res) {
    std::map<uint32_t, ResponseHeader::Handler>::const_iterator it = validResponseHeaders.find(hash);

    if (it == validResponseHeaders.end()) {
        return ;
    }
    method = it->second;
    return (this->*method)(res);
}

ResponseHeader::ResponseHeader(std::string keyToSet) : key(keyToSet) { }

void
ResponseHeader::setKey(std::string &keyToSet) {
    key = keyToSet;
}


void
ResponseHeader::AcceptPatch(Response &res) {
    (void)res;
}

void
ResponseHeader::AcceptRanges(Response &res) {
    (void)res;
}

void
ResponseHeader::Age(Response &res) {
    (void)res;
}

void
ResponseHeader::Allow(Response &res) {
    (void)res;
}

void
ResponseHeader::AltSvc(Response &res) {
    (void)res;
}

void
ResponseHeader::CacheControl(Response &res) {
    (void)res;
}

void
ResponseHeader::Connection(Response &res) {
    if (value.empty() == false) {
        return ;
    }
    if (res.getRequest()->getHeaderValue(CONNECTION) == "close") {
        res.shouldBeClosed(true);
        value = "close";
    } else {
        value = "keep-alive";
    }
}

void
ResponseHeader::ContentDisposition(Response &res) {
    (void)res;
}

void
ResponseHeader::ContentEncoding(Response &res) {
    (void)res;
}

void
ResponseHeader::ContentLanguage(Response &res) {
    (void)res;
}

void
ResponseHeader::ContentLength(Response &res) {
    if (value.empty() == false) {
        return ;
    }
    if (res.getBody().size()) {
        value = to_string(res.getBody().size());
    }
}

void
ResponseHeader::ContentLocation(Response &res) {
    (void)res;
}

void
ResponseHeader::ContentRange(Response &res) {
    (void)res;
}

void
ResponseHeader::ContentType(Response &res) {
    (void)res;
}

void
ResponseHeader::Date(Response &res) {
    (void)res;
}

void
ResponseHeader::DeltaBase(Response &res) {
    (void)res;
}

void
ResponseHeader::Expires(Response &res) {
    (void)res;
}

void
ResponseHeader::ETag(Response &res) {
    (void)res;
}

void
ResponseHeader::IM(Response &res) {
    (void)res;
}

void
ResponseHeader::Host(Response &res) {
    (void)res;
}

void
ResponseHeader::KeepAlive(Response &res) {
    (void)res;
    value = "timeout=55, max=1000";
}

void
ResponseHeader::LastModified(Response &res) {
    (void)res;
}

void
ResponseHeader::Link(Response &res) {
    (void)res;
}

void
ResponseHeader::Location(Response &res) {
    (void)res;
}

void
ResponseHeader::Pragma(Response &res) {
    (void)res;
}

void
ResponseHeader::ProxyAuthenticate(Response &res) {
    (void)res;
}

void
ResponseHeader::PublicKeyPins(Response &res) {
    (void)res;
}

void
ResponseHeader::RetryAfter(Response &res) {
    (void)res;
}

void
ResponseHeader::Server(Response &res) {
    (void)res;
}

void
ResponseHeader::SetCookie(Response &res) {
    (void)res;
}

void
ResponseHeader::StrictTransportSecurity(Response &res) {
    (void)res;
}

void
ResponseHeader::Trailer(Response &res) {
    (void)res;
}

void
ResponseHeader::TransferEncoding(Response &res) {
    (void)res;
}

void
ResponseHeader::Tk(Response &res) {
    (void)res;
}

void
ResponseHeader::Upgrade(Response &res) {
    (void)res;
}

void
ResponseHeader::Vary(Response &res) {
    (void)res;
}

void
ResponseHeader::Via(Response &res) {
    (void)res;
}

void
ResponseHeader::Warning(Response &res) {
    (void)res;
}

void
ResponseHeader::WWWAuthenticate(Response &res) {
    (void)res;
}

// Non-standard
void
ResponseHeader::ContentSecurityPolicy(Response &res) {
    (void)res;
}

void
ResponseHeader::Refresh(Response &res) {
    (void)res;
}

void
ResponseHeader::XPoweredBy(Response &res) {
    (void)res;
}

void
ResponseHeader::XRequestID(Response &res) {
    (void)res;
}

void
ResponseHeader::XUACompatible(Response &res) {
    (void)res;
}

void
ResponseHeader::XXSSProtection(Response &res) {
    (void)res;
}

// CORS
void
ResponseHeader::AccessControlAllowOrigin(Response &res) {
    (void)res;
}

void
ResponseHeader::AccessControlAllowCredentials(Response &res) {
    (void)res;
}

void
ResponseHeader::AccessControlExposeHeaders(Response &res) {
    (void)res;
}

void
ResponseHeader::AccessControlMaxAge(Response &res) {
    (void)res;
}

void
ResponseHeader::AccessControlAllowMethods(Response &res) {
    (void)res;
}

void
ResponseHeader::AccessControlAllowHeaders(Response &res) {
    (void)res;
}

void
ResponseHeader::NotSupported(Response &res) {
    (void)res;
}


static std::map<uint32_t, ResponseHeader::Handler>
initResponseHeadersMap(void) {
    std::map<uint32_t, ResponseHeader::Handler> tmp;

    tmp.insert(std::make_pair(A_IM, &ResponseHeader::AcceptPatch));
    tmp.insert(std::make_pair(ACCEPT, &ResponseHeader::AcceptRanges));
    tmp.insert(std::make_pair(ACCEPT_CHARSET, &ResponseHeader::Age));
    tmp.insert(std::make_pair(ACCEPT_ENCODING, &ResponseHeader::Allow));
    tmp.insert(std::make_pair(ACCEPT_LANGUAGE, &ResponseHeader::AltSvc));
    tmp.insert(std::make_pair(CACHE_CONTROL, &ResponseHeader::CacheControl));
    tmp.insert(std::make_pair(CONNECTION, &ResponseHeader::Connection));
    tmp.insert(std::make_pair(COOKIE, &ResponseHeader::ContentDisposition));
    tmp.insert(std::make_pair(ACCEPT_DATETIME, &ResponseHeader::ContentEncoding));
    tmp.insert(std::make_pair(ACCESS_CONTROL_REQUEST_METHOD, &ResponseHeader::ContentLanguage));
    tmp.insert(std::make_pair(CONTENT_LENGTH, &ResponseHeader::ContentLength));
    tmp.insert(std::make_pair(ACCESS_CONTROL_REQUEST_HEADERS, &ResponseHeader::ContentLocation));
    tmp.insert(std::make_pair(AUTHORIZATION, &ResponseHeader::ContentRange));
    tmp.insert(std::make_pair(CONTENT_TYPE, &ResponseHeader::ContentType));
    tmp.insert(std::make_pair(DATE, &ResponseHeader::Date));
    tmp.insert(std::make_pair(EXPECT, &ResponseHeader::DeltaBase));
    tmp.insert(std::make_pair(FORWARDED, &ResponseHeader::Expires));
    tmp.insert(std::make_pair(FROM, &ResponseHeader::ETag));
    tmp.insert(std::make_pair(IF_MATCH, &ResponseHeader::IM));
    tmp.insert(std::make_pair(IF_MODIFIED_SINCE, &ResponseHeader::KeepAlive));
    tmp.insert(std::make_pair(HOST, &ResponseHeader::Host));
    tmp.insert(std::make_pair(IF_NONE_MATCH, &ResponseHeader::LastModified));
    tmp.insert(std::make_pair(IF_RANGE, &ResponseHeader::Link));
    tmp.insert(std::make_pair(IF_UNMODIFIED_SINCE, &ResponseHeader::Location));
    tmp.insert(std::make_pair(PRAGMA, &ResponseHeader::Pragma));
    tmp.insert(std::make_pair(MAX_FORWARDS, &ResponseHeader::ProxyAuthenticate));
    tmp.insert(std::make_pair(ORIGIN, &ResponseHeader::PublicKeyPins));
    tmp.insert(std::make_pair(PROXY_AUTHORIZATION, &ResponseHeader::RetryAfter));
    tmp.insert(std::make_pair(RANGE, &ResponseHeader::Server));
    tmp.insert(std::make_pair(REFERER, &ResponseHeader::SetCookie));
    tmp.insert(std::make_pair(TE, &ResponseHeader::StrictTransportSecurity));
    tmp.insert(std::make_pair(USER_AGENT, &ResponseHeader::Trailer));
    tmp.insert(std::make_pair(TRANSFER_ENCODING, &ResponseHeader::TransferEncoding));
    tmp.insert(std::make_pair(DNT, &ResponseHeader::Tk));
    tmp.insert(std::make_pair(UPGRADE, &ResponseHeader::Upgrade));
    tmp.insert(std::make_pair(X_REQUESTED_WITH, &ResponseHeader::Vary));
    tmp.insert(std::make_pair(VIA, &ResponseHeader::Via));
    tmp.insert(std::make_pair(WARNING, &ResponseHeader::Warning));
    tmp.insert(std::make_pair(X_CSRF_TOKEN, &ResponseHeader::WWWAuthenticate));

    tmp.insert(std::make_pair(SEC_FETCH_DEST, &ResponseHeader::ContentSecurityPolicy));
    tmp.insert(std::make_pair(SEC_FETCH_MODE, &ResponseHeader::Refresh));
    tmp.insert(std::make_pair(SEC_FETCH_SITE, &ResponseHeader::XPoweredBy));
    tmp.insert(std::make_pair(SEC_FETCH_USER, &ResponseHeader::XRequestID));
    tmp.insert(std::make_pair(UPGRADE_INSECURE_REQUESTS, &ResponseHeader::XUACompatible));
    tmp.insert(std::make_pair(SEC_CH_UA, &ResponseHeader::XXSSProtection));

    tmp.insert(std::make_pair(SEC_GPC, &ResponseHeader::AccessControlAllowOrigin));
    tmp.insert(std::make_pair(SEC_CH_UA_MOBILE, &ResponseHeader::AccessControlAllowCredentials));
    tmp.insert(std::make_pair(SEC_CH_UA_PLATFORM, &ResponseHeader::AccessControlExposeHeaders));
    tmp.insert(std::make_pair(SEC_GPC, &ResponseHeader::AccessControlMaxAge));
    tmp.insert(std::make_pair(SEC_CH_UA_MOBILE, &ResponseHeader::AccessControlAllowMethods));
    tmp.insert(std::make_pair(SEC_CH_UA_PLATFORM, &ResponseHeader::AccessControlAllowHeaders));

    tmp.insert(std::make_pair(PURPOSE, &ResponseHeader::NotSupported));

    return tmp;
}

const std::map<uint32_t, ResponseHeader::Handler> validResponseHeaders = initResponseHeadersMap();

}