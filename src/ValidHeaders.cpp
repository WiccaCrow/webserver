#include "ValidHeaders.hpp"

namespace HTTP {

std::map<uint32_t, Header::Handler> createHeadersMap(void) {
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

const std::map<uint32_t, Header::Handler> validHeaders = createHeadersMap();

} // namespace HTTP
