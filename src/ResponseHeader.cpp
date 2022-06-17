#include "ResponseHeader.hpp"
#include "Client.hpp"

namespace HTTP {

void
ResponseHeader::handle(Response &res) {
    std::map<uint32_t, ResponseHeader::Handler>::const_iterator it;
    it = validResHeaders.find(hash);

    if (it == validResHeaders.end()) {
        Log.debug() << "RequestHeader:: Unknown header: " << headerNames[hash] << std::endl;
        Log.debug() << "RequestHeader:: Value: " << value << std::endl;
        Log.debug() << "RequestHeader:: Hash: " << hash << std::endl;
        return ;
    }
    method = it->second;
    return (this->*method)(res);
}

ResponseHeader::ResponseHeader() : Header() { }

ResponseHeader::ResponseHeader(uint32_t hash) : Header(hash) { }

ResponseHeader::ResponseHeader(uint32_t hash, const std::string &value) :  Header(hash, value) { }

bool
ResponseHeader::isValid(void) {
    return (validResHeaders.find(hash) != validResHeaders.end());
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
    if (res.getRequest()->getMethod() == "OPTIONS") {
        std::vector<std::string> &allowedMethods = res.getRequest()->getLocation()->getAllowedMethodsRef();
        for (int i = 0, nbMetods = allowedMethods.size(); i < nbMetods;) {
            value += allowedMethods[i];
            if (++i != nbMetods) {
                value += ", ";
            }
        }
    }
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

    // res.getRequest()->getStatus() != MOVED_PERMANENTLY && 
    if (res.getRequest()->getHeaderValue(CONNECTION) == "close") {
        res.getClient()->shouldBeClosed(true);
        value = "close";
    } else {
        // res.getClient()->shouldBeClosed(false);
        value = "keep-alive";
        ResponseHeader *ptr = res.getHeader(KEEP_ALIVE);
        ptr->handle(res);
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
    if (!value.empty()) {
        return ;
    }
    if (res.getBodyLength() != 0) {
        value = to_string(res.getBodyLength());
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
    value = getDateTimeGMT();
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
    ResponseHeader *ptr = res.getHeader(CONNECTION);
    if (ptr && ptr->value != "close") {
        value = "timeout=55, max=1000";
    }
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
    value = SERVER_SOFTWARE;
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
    value = "Basic realm=\"" + res.getRequest()->getLocation()->getAuthRef().getRealmRef() + "\"";
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

    tmp.insert(std::make_pair(ACCEPT_PATCH, &ResponseHeader::AcceptPatch));
    tmp.insert(std::make_pair(ACCEPT_RANGES, &ResponseHeader::AcceptRanges));
    tmp.insert(std::make_pair(AGE, &ResponseHeader::Age));
    tmp.insert(std::make_pair(ALLOW, &ResponseHeader::Allow));
    tmp.insert(std::make_pair(ALT_SVC, &ResponseHeader::AltSvc));
    tmp.insert(std::make_pair(CACHE_CONTROL, &ResponseHeader::CacheControl));
    tmp.insert(std::make_pair(CONNECTION, &ResponseHeader::Connection));
    tmp.insert(std::make_pair(CONTENT_DISPOSITION, &ResponseHeader::ContentDisposition));
    tmp.insert(std::make_pair(CONTENT_ENCODING, &ResponseHeader::ContentEncoding));
    tmp.insert(std::make_pair(CONTENT_LANGUAGE, &ResponseHeader::ContentLanguage));
    tmp.insert(std::make_pair(CONTENT_LENGTH, &ResponseHeader::ContentLength));
    tmp.insert(std::make_pair(CONTENT_LOCATION, &ResponseHeader::ContentLocation));
    tmp.insert(std::make_pair(CONTENT_RANGE, &ResponseHeader::ContentRange));
    tmp.insert(std::make_pair(CONTENT_TYPE, &ResponseHeader::ContentType));
    tmp.insert(std::make_pair(DATE, &ResponseHeader::Date));
    tmp.insert(std::make_pair(DELTA_BASE, &ResponseHeader::DeltaBase));
    tmp.insert(std::make_pair(EXPIRES, &ResponseHeader::Expires));
    tmp.insert(std::make_pair(ETAG, &ResponseHeader::ETag));
    tmp.insert(std::make_pair(IM, &ResponseHeader::IM));
    tmp.insert(std::make_pair(KEEP_ALIVE, &ResponseHeader::KeepAlive));
    tmp.insert(std::make_pair(HOST, &ResponseHeader::Host));
    tmp.insert(std::make_pair(LAST_MODIFIED, &ResponseHeader::LastModified));
    tmp.insert(std::make_pair(LINK, &ResponseHeader::Link));
    tmp.insert(std::make_pair(LOCATION, &ResponseHeader::Location));
    tmp.insert(std::make_pair(PRAGMA, &ResponseHeader::Pragma));
    tmp.insert(std::make_pair(PROXY_AUTHENTICATE, &ResponseHeader::ProxyAuthenticate));
    tmp.insert(std::make_pair(PUBLIC_KEY_PINS, &ResponseHeader::PublicKeyPins));
    tmp.insert(std::make_pair(RETRY_AFTER, &ResponseHeader::RetryAfter));
    tmp.insert(std::make_pair(SERVER, &ResponseHeader::Server));
    tmp.insert(std::make_pair(SET_COOKIE, &ResponseHeader::SetCookie));
    tmp.insert(std::make_pair(STRICT_TRANSPORT_SECURITY, &ResponseHeader::StrictTransportSecurity));
    tmp.insert(std::make_pair(TRAILER, &ResponseHeader::Trailer));
    tmp.insert(std::make_pair(TRANSFER_ENCODING, &ResponseHeader::TransferEncoding));
    tmp.insert(std::make_pair(TK, &ResponseHeader::Tk));
    tmp.insert(std::make_pair(UPGRADE, &ResponseHeader::Upgrade));
    tmp.insert(std::make_pair(VARY, &ResponseHeader::Vary));
    tmp.insert(std::make_pair(VIA, &ResponseHeader::Via));
    tmp.insert(std::make_pair(WARNING, &ResponseHeader::Warning));
    tmp.insert(std::make_pair(WWW_AUTHENTICATE, &ResponseHeader::WWWAuthenticate));
    tmp.insert(std::make_pair(CONTENT_SECURITY_POLICY, &ResponseHeader::ContentSecurityPolicy));
    tmp.insert(std::make_pair(REFRESH, &ResponseHeader::Refresh));
    tmp.insert(std::make_pair(X_POWERED_BY, &ResponseHeader::XPoweredBy));
    tmp.insert(std::make_pair(X_REQUEST_ID, &ResponseHeader::XRequestID));
    tmp.insert(std::make_pair(X_UA_COMPATIBLE, &ResponseHeader::XUACompatible));
    tmp.insert(std::make_pair(X_XSS_PROTECTION, &ResponseHeader::XXSSProtection));
    tmp.insert(std::make_pair(ACCESS_CONTROL_ALLOW_ORIGIN, &ResponseHeader::AccessControlAllowOrigin));
    tmp.insert(std::make_pair(ACCESS_CONTROL_ALLOW_CREDENTIALS, &ResponseHeader::AccessControlAllowCredentials));
    tmp.insert(std::make_pair(ACCESS_CONTROL_EXPOSE_HEADERS, &ResponseHeader::AccessControlExposeHeaders));
    tmp.insert(std::make_pair(ACCESS_CONTROL_MAX_AGE, &ResponseHeader::AccessControlMaxAge));
    tmp.insert(std::make_pair(ACCESS_CONTROL_ALLOW_METHODS, &ResponseHeader::AccessControlAllowMethods));
    tmp.insert(std::make_pair(ACCESS_CONTROL_ALLOW_HEADERS, &ResponseHeader::AccessControlAllowHeaders));

    return tmp;
}

const std::map<uint32_t, ResponseHeader::Handler> validResHeaders = initResponseHeadersMap();

}