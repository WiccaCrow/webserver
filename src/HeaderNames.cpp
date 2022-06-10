#include "HeaderNames.hpp"

namespace HTTP {

HeaderNames::HeaderNames() {

    _headerNames.insert(std::make_pair(A_IM, "a-im"));
    _headerNames.insert(std::make_pair(ACCEPT, "accept"));
    _headerNames.insert(std::make_pair(ACCEPT_CHARSET, "accept-charset"));
    _headerNames.insert(std::make_pair(ACCEPT_ENCODING, "accept-encoding"));
    _headerNames.insert(std::make_pair(ACCEPT_LANGUAGE, "accept-language"));
    _headerNames.insert(std::make_pair(ACCEPT_DATETIME, "accept-datetime"));
    _headerNames.insert(std::make_pair(ACCESS_CONTROL_REQUEST_METHOD, "access-control-request-method"));
    _headerNames.insert(std::make_pair(ACCESS_CONTROL_REQUEST_HEADERS, "access-control-request-headers"));
    _headerNames.insert(std::make_pair(AUTHORIZATION, "authorization"));
    _headerNames.insert(std::make_pair(CACHE_CONTROL, "cache-control"));
    _headerNames.insert(std::make_pair(CONNECTION, "connection"));
    _headerNames.insert(std::make_pair(CONTENT_LENGTH, "content-length"));
    _headerNames.insert(std::make_pair(CONTENT_TYPE, "content-type"));
    _headerNames.insert(std::make_pair(COOKIE, "cookie"));
    _headerNames.insert(std::make_pair(DATE, "date"));
    _headerNames.insert(std::make_pair(EXPECT, "expect"));
    _headerNames.insert(std::make_pair(FORWARDED, "forwarded"));
    _headerNames.insert(std::make_pair(FROM, "from"));
    _headerNames.insert(std::make_pair(HOST, "host"));
    _headerNames.insert(std::make_pair(IF_MATCH, "if-match"));
    _headerNames.insert(std::make_pair(IF_MODIFIED_SINCE, "if-modified-since"));
    _headerNames.insert(std::make_pair(IF_NONE_MATCH, "if-none-match"));
    _headerNames.insert(std::make_pair(IF_RANGE, "if-range"));
    _headerNames.insert(std::make_pair(IF_UNMODIFIED_SINCE, "if-unmodified-since"));
    _headerNames.insert(std::make_pair(KEEP_ALIVE, "keep-alive"));
    _headerNames.insert(std::make_pair(MAX_FORWARDS, "max-forwards"));
    _headerNames.insert(std::make_pair(ORIGIN, "origin"));
    _headerNames.insert(std::make_pair(PRAGMA, "pragma"));
    _headerNames.insert(std::make_pair(PROXY_AUTHORIZATION, "proxy-authorization"));
    _headerNames.insert(std::make_pair(RANGE, "range"));
    _headerNames.insert(std::make_pair(REFERER, "referer"));
    _headerNames.insert(std::make_pair(TRANSFER_ENCODING, "transfer-encoding"));
    _headerNames.insert(std::make_pair(TE, "te"));
    _headerNames.insert(std::make_pair(USER_AGENT, "user-agent"));
    _headerNames.insert(std::make_pair(UPGRADE, "upgrade"));
    _headerNames.insert(std::make_pair(VIA, "via"));
    _headerNames.insert(std::make_pair(WARNING, "warning"));
    _headerNames.insert(std::make_pair(DNT, "dnt"));
    _headerNames.insert(std::make_pair(X_REQUESTED_WITH, "x-requested-with"));
    _headerNames.insert(std::make_pair(X_CSRF_TOKEN, "x-csrf-token"));
    _headerNames.insert(std::make_pair(SEC_FETCH_DEST, "sec-fetch-dest"));
    _headerNames.insert(std::make_pair(SEC_FETCH_MODE, "sec-fetch-mode"));
    _headerNames.insert(std::make_pair(SEC_FETCH_SITE, "sec-fetch-site"));
    _headerNames.insert(std::make_pair(SEC_FETCH_USER, "sec-fetch-user"));
    _headerNames.insert(std::make_pair(UPGRADE_INSECURE_REQUESTS, "upgrade-insecure-requests"));
    _headerNames.insert(std::make_pair(SEC_CH_UA, "sec-ch-ua"));
    _headerNames.insert(std::make_pair(SEC_GPC, "sec-gpc"));
    _headerNames.insert(std::make_pair(SEC_CH_UA_MOBILE, "sec-ch-ua-mobile"));
    _headerNames.insert(std::make_pair(SEC_CH_UA_PLATFORM, "sec-ch-ua-platform"));
    _headerNames.insert(std::make_pair(PURPOSE, "purpose"));
    _headerNames.insert(std::make_pair(ACCEPT_PATCH, "accept-patch"));
    _headerNames.insert(std::make_pair(ACCEPT_RANGES, "accept-ranges"));
    _headerNames.insert(std::make_pair(AGE, "age"));
    _headerNames.insert(std::make_pair(ALLOW, "allow"));
    _headerNames.insert(std::make_pair(ALT_SVC, "alt-svc"));
    _headerNames.insert(std::make_pair(CONTENT_DISPOSITION, "content-disposition"));
    _headerNames.insert(std::make_pair(CONTENT_ENCODING, "content-encoding"));
    _headerNames.insert(std::make_pair(CONTENT_LANGUAGE, "content-language"));
    _headerNames.insert(std::make_pair(CONTENT_LOCATION, "content-location"));
    _headerNames.insert(std::make_pair(CONTENT_RANGE, "content-range"));
    _headerNames.insert(std::make_pair(DELTA_BASE, "delta-base"));
    _headerNames.insert(std::make_pair(ETAG, "etag"));
    _headerNames.insert(std::make_pair(EXPIRES, "expires"));
    _headerNames.insert(std::make_pair(IM, "im"));
    _headerNames.insert(std::make_pair(LAST_MODIFIED, "last-modified"));
    _headerNames.insert(std::make_pair(LINK, "link"));
    _headerNames.insert(std::make_pair(LOCATION, "location"));
    _headerNames.insert(std::make_pair(PROXY_AUTHENTICATE, "proxy-authenticate"));
    _headerNames.insert(std::make_pair(PUBLIC_KEY_PINS, "public-key-pins"));
    _headerNames.insert(std::make_pair(RETRY_AFTER, "retry-after"));
    _headerNames.insert(std::make_pair(SERVER, "server"));
    _headerNames.insert(std::make_pair(SET_COOKIE, "set-cookie"));
    _headerNames.insert(std::make_pair(STRICT_TRANSPORT_SECURITY, "strict-transport-security"));
    _headerNames.insert(std::make_pair(TRAILER, "trailer"));
    _headerNames.insert(std::make_pair(TK, "tk"));
    _headerNames.insert(std::make_pair(VARY, "vary"));
    _headerNames.insert(std::make_pair(WWW_AUTHENTICATE, "www-authenticate"));
    _headerNames.insert(std::make_pair(ACCESS_CONTROL_ALLOW_ORIGIN, "access-control-allow-origin"));
    _headerNames.insert(std::make_pair(ACCESS_CONTROL_ALLOW_CREDENTIALS, "access-control-allow-credentials"));
    _headerNames.insert(std::make_pair(ACCESS_CONTROL_EXPOSE_HEADERS, "access-control-expose-headers"));
    _headerNames.insert(std::make_pair(ACCESS_CONTROL_MAX_AGE, "access-control-max-age"));
    _headerNames.insert(std::make_pair(ACCESS_CONTROL_ALLOW_METHODS, "access-control-allow-methods"));
    _headerNames.insert(std::make_pair(ACCESS_CONTROL_ALLOW_HEADERS, "access-control-allow-headers"));
    _headerNames.insert(std::make_pair(CONTENT_SECURITY_POLICY, "content-security-policy"));
    _headerNames.insert(std::make_pair(REFRESH, "refresh"));
    _headerNames.insert(std::make_pair(X_POWERED_BY, "x-powered-by"));
    _headerNames.insert(std::make_pair(X_REQUEST_ID, "x-request-id"));
    _headerNames.insert(std::make_pair(X_UA_COMPATIBLE, "x-ua-compatible"));
    _headerNames.insert(std::make_pair(X_XSS_PROTECTION, "x-xss-protection"));
}

HeaderNames::~HeaderNames() {}

const std::string &HeaderNames::operator[](HeaderCode code) const {
    std::map<uint32_t, std::string>::const_iterator it = _headerNames.find(code);
    if (it == _headerNames.end()) {
        return _empty;
    }
    return it->second;
}

const std::string &HeaderNames::operator[](uint32_t code) const {
    return this->operator[](static_cast<HeaderCode>(code));
}

const HeaderNames headerNames;

};