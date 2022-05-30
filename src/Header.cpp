#include "Header.hpp"
#include "Request.hpp"

namespace HTTP {

StatusCode Header::handle(Request &req) {
    std::map<uint32_t, Header::Handler>::iterator it = validHeaders.find(hash);
    
    if (it == validHeaders.end()) {
        // Or bad request
        return BAD_REQUEST;
    }
    method = it->second;
    return (this->*method)(req);
}

StatusCode
Header::A_IM(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Accept(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::AcceptCharset(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::AcceptEncoding(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::AcceptLanguage(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::AcceptDateTime(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::AccessControlRequestMethod(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::AccessControlRequestHeaders(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Authorization(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::CacheControl(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Connection(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Cookie(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Date(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Expect(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Forwarded(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::From(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Host(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::IfMatch(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::IfModifiedSince(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::IfNoneMatch(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::IfRange(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::IfUnmodifiedSince(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::MaxForwards(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Origin(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Pragma(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::ProxyAuthorization(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Range(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Referer(Request &req) {
    URI ref;
    ref.parse(getVal());

    req.getUriRef()._path = ref._path + req.getUriRef()._path;
    return CONTINUE;
}

StatusCode
Header::TransferEncoding(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::TE(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::UserAgent(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::ContentLength(Request &req) {
    (void)req;
    return CONTINUE;
}

// StatusCode Header::SetCookie(Request &req) {
//     (void)req;
//     return CONTINUE;
// }

StatusCode
Header::ContentType(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Upgrade(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Via(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Warning(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::Dnt(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::XRequestedWith(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::XCsrfToken(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::SecFetchDest(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::SecFetchMode(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::SecFetchSite(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::SecFetchUser(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::UpgradeInsecureRequests(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::SecChUa(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::SecGpc(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::SecChUaMobile(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::SecChUaPlatform(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
Header::NotSupported(Request &req) {
    (void)req;
    return CONTINUE;
}

std::map<uint32_t, Header::Handler>
createHeadersMap(void) {
    std::map<uint32_t, Header::Handler> tmp;

    tmp.insert(std::make_pair(A_IM, &Header::A_IM));
    tmp.insert(std::make_pair(ACCEPT, &Header::Accept));
    tmp.insert(std::make_pair(ACCEPT_CHARSET, &Header::AcceptCharset));
    tmp.insert(std::make_pair(ACCEPT_ENCODING, &Header::AcceptEncoding));
    tmp.insert(std::make_pair(ACCEPT_LANGUAGE, &Header::AcceptLanguage));
    tmp.insert(std::make_pair(ACCEPT_DATETIME, &Header::AcceptDateTime));
    tmp.insert(std::make_pair(ACCESS_CONTROL_REQUEST_METHOD, &Header::AccessControlRequestMethod));
    tmp.insert(std::make_pair(ACCESS_CONTROL_REQUEST_HEADERS, &Header::AccessControlRequestHeaders));
    tmp.insert(std::make_pair(AUTHORIZATION, &Header::Authorization));
    tmp.insert(std::make_pair(CACHE_CONTROL, &Header::CacheControl));
    tmp.insert(std::make_pair(CONNECTION, &Header::Connection));
    tmp.insert(std::make_pair(CONTENT_LENGTH, &Header::ContentLength));
    tmp.insert(std::make_pair(CONTENT_TYPE, &Header::ContentType));
    tmp.insert(std::make_pair(COOKIE, &Header::Cookie));
    tmp.insert(std::make_pair(DATE, &Header::Date));
    tmp.insert(std::make_pair(EXPECT, &Header::Expect));
    tmp.insert(std::make_pair(FORWARDED, &Header::Forwarded));
    tmp.insert(std::make_pair(FROM, &Header::From));
    tmp.insert(std::make_pair(HOST, &Header::Host));
    tmp.insert(std::make_pair(IF_MATCH, &Header::IfMatch));
    tmp.insert(std::make_pair(IF_MODIFIED_SINCE, &Header::IfModifiedSince));
    tmp.insert(std::make_pair(IF_NONE_MATCH, &Header::IfNoneMatch));
    tmp.insert(std::make_pair(IF_RANGE, &Header::IfRange));
    tmp.insert(std::make_pair(IF_UNMODIFIED_SINCE, &Header::IfUnmodifiedSince));
    tmp.insert(std::make_pair(MAX_FORWARDS, &Header::MaxForwards));
    tmp.insert(std::make_pair(ORIGIN, &Header::Origin));
    tmp.insert(std::make_pair(PRAGMA, &Header::Pragma));
    tmp.insert(std::make_pair(PROXY_AUTHORIZATION, &Header::ProxyAuthorization));
    tmp.insert(std::make_pair(RANGE, &Header::Range));
    tmp.insert(std::make_pair(REFERER, &Header::Referer));
    tmp.insert(std::make_pair(TRANSFER_ENCODING, &Header::TransferEncoding));
    tmp.insert(std::make_pair(TE, &Header::TE));
    tmp.insert(std::make_pair(USER_AGENT, &Header::UserAgent));
    tmp.insert(std::make_pair(UPGRADE, &Header::Upgrade));
    tmp.insert(std::make_pair(VIA, &Header::Via));
    tmp.insert(std::make_pair(WARNING, &Header::Warning));
    tmp.insert(std::make_pair(DNT, &Header::Dnt));
    tmp.insert(std::make_pair(X_REQUESTED_WITH, &Header::XRequestedWith));
    tmp.insert(std::make_pair(X_CSRF_TOKEN, &Header::XCsrfToken));

    tmp.insert(std::make_pair(SEC_FETCH_DEST, &Header::SecFetchDest));
    tmp.insert(std::make_pair(SEC_FETCH_MODE, &Header::SecFetchMode));
    tmp.insert(std::make_pair(SEC_FETCH_SITE, &Header::SecFetchSite));
    tmp.insert(std::make_pair(SEC_FETCH_USER, &Header::SecFetchUser));
    tmp.insert(std::make_pair(UPGRADE_INSECURE_REQUESTS, &Header::UpgradeInsecureRequests));
    tmp.insert(std::make_pair(SEC_CH_UA, &Header::SecChUa));
    tmp.insert(std::make_pair(SEC_GPC, &Header::SecGpc));
    tmp.insert(std::make_pair(SEC_CH_UA_MOBILE, &Header::SecChUaMobile));
    tmp.insert(std::make_pair(SEC_CH_UA_PLATFORM, &Header::SecChUaPlatform));

    tmp.insert(std::make_pair(PURPOSE, &Header::NotSupported));

    return tmp;
}


std::map<uint32_t, Header::Handler> validHeaders = createHeadersMap();

}
