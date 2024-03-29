#include "RequestHeader.hpp"
#include "Server.hpp"

namespace HTTP {

StatusCode
RequestHeader::handle(Request &req) {
    std::map<uint32_t, RequestHeader::Handler>::const_iterator it = validReqHeaders.find(hash);

    if (it == validReqHeaders.end()) {
        Log.debug() << "RequestHeader:: Unknown header: " << key << Log.endl;
        Log.debug() << "RequestHeader:: Value: " << value << Log.endl;
        Log.debug() << "RequestHeader:: Hash: " << ultos(hash) << Log.endl;
        return CONTINUE;
    }
    method = it->second;
    return (this->*method)(req);
}

bool
RequestHeader::isValid(void) {
    return (validReqHeaders.find(hash) != validReqHeaders.end());
}

StatusCode
RequestHeader::A_IM(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::Accept(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::AcceptCharset(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::AcceptEncoding(Request &req) {
    (void)req;

    // std::vector<std::string> encodings = split(value, ",");

    // gzip compress deflate br identity *
    // for (std::size_t i = 0; i < encodings.size(); i++) {
    //     trim(encodings[i], SP CRLF);
    //     std::string encoding = encodings[i];
    //     std::size_t pos = encodings[i].find(';');
    //     if (pos == std::string::npos) {
    //         encoding = encodings[i].substr(0, pos);
    //     }
        // if (encoding != "gzip" &&
        //     encoding != "compress" &&
        //     encoding != "deflate" &&
        //     encoding != "br" &&
        //     encoding != "identity" &&
        //     encoding != "*") {
        //     return NOT_ACCEPTABLE;
        // }
    // }
    return CONTINUE;
}

StatusCode
RequestHeader::AcceptLanguage(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::AcceptDateTime(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::AccessControlRequestMethod(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::AccessControlRequestHeaders(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::Authorization(Request &req) {

    if (req.isProxy()) {
        Log.debug() << "Authorization::Proxy-request received" << Log.endl;
        return CONTINUE;
    }

    const Auth &auth = req.getLocation()->getAuthRef();
    if (!auth.isSet()) {
        Log.debug() << "Authorization::Auth not set" << Log.endl;
        return CONTINUE;
    }

    Log.debug() << "Authorization::" << value << Log.endl;
    std::vector<std::string> splitted = split(value, " ");

    if (splitted[0] != "Basic") {
        Log.debug() << "Authorization::Only Basic supported for now " <<  splitted[0] << Log.endl;
        return NOT_IMPLEMENTED;
    }

    std::string decoded = Base64::decode(splitted[1]);
    Log.debug() << "Authorization::Decoded Base64:" << decoded << Log.endl;
    if (decoded.empty()) {
        Log.debug() << "Authorization::Invalid Base64 string" << Log.endl;
        return UNAUTHORIZED;
    }

    req.authorized(auth.isAuthorized(decoded, &req));
    if (req.authorized()) {
        Log.debug() << "Authorization::Succeed" << Log.endl;
    } else {
        Log.debug() << "Authorization::Failed" << Log.endl;
    }
    
    return CONTINUE;
}

StatusCode
RequestHeader::CacheControl(Request &req) {
    (void)req;
    return CONTINUE;
}

// All connections are considered as 'keep-alive' unless declared 'closed'
// In the current version, the list of 'hop-by-hop' directives (if exists in value) are deleted and ignored
StatusCode
RequestHeader::Connection(Request &req) {
    (void)req;

    toLowerCase(value);
    if (value.find("close") != std::string::npos) {
        value = "close";
    }
    return CONTINUE;
}

StatusCode
RequestHeader::ContentLength(Request &req) {
    if (req.has(TRANSFER_ENCODING)) {
        Log.debug() << "ContentLength::ContentLength: TransferEncoding header exist" << Log.endl;
        return BAD_REQUEST;
    }

    long long length;
    if (!stoll(length, value.c_str())) {
        return BAD_REQUEST;
    }

    if (static_cast<std::size_t>(length) > req.getLocation()->getPostMaxBodyRef()) {
        return PAYLOAD_TOO_LARGE;
    }

    req.setExpBodySize(length);

    return CONTINUE;
}

StatusCode
RequestHeader::ContentType(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::Cookie(Request &req) {
    std::map<std::string, std::string> cookie;

    std::vector<std::string> cookie_pairs = split(value, " ;");
    for (std::size_t i = 0; i < cookie_pairs.size(); ++i) {
        std::size_t colonPos = cookie_pairs[i].find('=');
        if (colonPos == std::string::npos) {
            continue;
        }
        std::string cookie_key   = cookie_pairs[i].substr(0, colonPos);
        std::string cookie_value = cookie_pairs[i].substr(colonPos + 1);
        cookie[cookie_key]       = cookie_value;
    }
    req.setCookie(cookie);
    return CONTINUE;
}

StatusCode
RequestHeader::Date(Request &req) {
    (void)req;
    // This header is mostly for web-servers, and client send it very rarely.
    return CONTINUE;
}

StatusCode
RequestHeader::Expect(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::Forwarded(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::From(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::Host(Request &req) {
    
    URI &uri = req.getUriRef();
    URI hdr;

    if (!uri._host.empty() && req.isProxy()) {
        return CONTINUE;
    }

    hdr.parse(value);
    if (!isValidHost(hdr._host)) {
        Log.error() << "Host: Invalid Host " << hdr._host << Log.endl;
        return BAD_REQUEST;
    }

    if (!uri._host.empty()) {
        if (hdr._host != uri._host) {
            Log.error() << "Host header doesn't match host in the uri" << Log.endl;
            return BAD_REQUEST;
        }
    }

    if (!uri._port_s.empty()) {
        if (hdr._port_s != uri._port_s) {
            Log.error() << "Port in the header doesn't match port in the uri" << Log.endl;
            return BAD_REQUEST;
        }
    }

    uri._host = hdr._host;
    uri._port = hdr._port;
    uri._port_s = hdr._port_s;
    
    return CONTINUE;
}

StatusCode
RequestHeader::IfMatch(Request &req) {

    std::vector<std::string> tags = split(value, ", \"");

    if (tags[0] == "*") {
        if (tags.size() == 1) {
            Log.debug() << "IfMatch:: Any allowed " << Log.endl;
            return CONTINUE;
        } else {
            Log.debug() << "IfMatch:: Invalid format: " << value << Log.endl;
            // Maybe not bad request
            return BAD_REQUEST;
        }
    }

    ETag *tag = ETag::get(req.getResolvedPath(), false);
    if (tag == NULL) {
        Log.debug() << "IfMatch:: No etag value found for " << req.getResolvedPath() << Log.endl;
        return PRECONDITION_FAILED;
    }

    std::vector<std::string>::iterator itMatched;
    itMatched = std::find(tags.begin(), tags.end(), tag->getTag());
    if (itMatched == tags.end()) {
        Log.debug() << "IfMatch:: None of etag values matched " << *itMatched << Log.endl;
        return PRECONDITION_FAILED;
    }
    return CONTINUE;
}

StatusCode
RequestHeader::IfNoneMatch(Request &req) {
    (void)req;

    std::vector<std::string> tags = split(value, ", \"");

    if (tags[0] == "*") {
        // Another handler
        if (tags.size() == 1) {
            Log.debug() << "IfNoneMatch:: Any allowed " << Log.endl;
            return CONTINUE;
        } else {
            Log.debug() << "IfNoneMatch:: Invalid format: " << value << Log.endl;
            // Maybe not bad request
            return BAD_REQUEST;
        }
    }

    ETag *tag = ETag::get(req.getResolvedPath(), false);
    if (tag == NULL) {
        Log.debug() << "IfNoneMatch:: [OK] No etag value found for " << req.getResolvedPath() << Log.endl;
        return CONTINUE;
    }

    std::vector<std::string>::iterator itMatched;
    itMatched = std::find(tags.begin(), tags.end(), tag->getTag());
    if (itMatched == tags.end()) {
        Log.debug() << "IfNoneMatch:: None of etag values matched " << *itMatched << Log.endl;
        return CONTINUE;
    }

    if (req.getMethod() == "GET" || req.getMethod() == "HEAD") {
        return NOT_MODIFIED;
    }
    return PRECONDITION_FAILED;
}

StatusCode
RequestHeader::IfModifiedSince(Request &req) {

    // A recipient MUST ignore If-Modified-Since if the request contains an
    // If-None-Match header field;
    if (req.has(IF_NONE_MATCH)) {
        // Maybe value should be cleared
        Log.debug() << "IfModifiedSince:: IfNoneMatch present" << Log.endl;
        return CONTINUE;
    }

    if (req.getMethod() == "GET" || req.getMethod() == "HEAD") {

        struct tm tm;
        if (!Time::gmt(value, &tm)) {
            Log.debug() << "IfModifiedSince:: Cannot read datetime " << value << Log.endl;
            // Server should ignore in case of invalid date - RFC 7232 (was BAD_REQUEST)
            // Maybe value should be cleared
            return CONTINUE;
        }

        // A date which is later than the server's current time is invalid. Add
        struct tm *cur = Time::gmtime();        
        if (Time::operator>(tm, *cur)) {
            return BAD_REQUEST;
        }
        
        if (Time::gmt(getModifiedTime(req.getResolvedPath())) == value) {
            Log.debug() << "IfModifiedSince:: 304 returned for " << req.getResolvedPath() << Log.endl;
            return NOT_MODIFIED;
        }
    }

    return CONTINUE;
}

StatusCode
RequestHeader::IfUnmodifiedSince(Request &req) {

    // A recipient MUST ignore If-Modified-Since if the request contains an
    // If-None-Match header field;
    if (req.has(IF_MATCH)) {
        // Maybe value should be cleared
        Log.debug() << "IfUnModifiedSince:: IfMatch present" << Log.endl;
        return CONTINUE;
    }

    struct tm tm;
    if (!Time::gmt(value, &tm)) {
        Log.debug() << "IfUnmodifiedSince:: Cannot read datetime " << value << Log.endl;
        return BAD_REQUEST;
    }

    if (Time::gmt(getModifiedTime(req.getResolvedPath())) != value) {
        Log.debug() << "IfUnmodifiedSince:: 412 returned for " << req.getResolvedPath() << Log.endl;
        return PRECONDITION_FAILED;
    }

    return CONTINUE;
}

StatusCode
RequestHeader::IfRange(Request &req) {

    if (req.isProxy()) {
        return CONTINUE;
    }

    struct tm tm;
    if (Time::gmt(value, &tm)) {

        ETag *tag = ETag::get(req.getResolvedPath());
        if (tag->getEntityStrTime() != value) {
            req.useRanges(false);
        }
        return CONTINUE;

    } else {
        Log.debug() << "IfRange:: Cannot read datetime, trying etag..." << value << Log.endl;
    }

    if (!Base64::isValid(value)) {
        Log.debug() << "IfRange:: Invalid etag " << value << Log.endl;
        return BAD_REQUEST;
    }

    ETag *tag = ETag::get(req.getResolvedPath());
    if (tag->getTag() != value) {
        req.useRanges(false);
    }
    
    return CONTINUE;
}

StatusCode
RequestHeader::KeepAlive(Request &req) {
    (void)req;
    // Not sure, but should be used by servers mostly
    return CONTINUE;
}
    
StatusCode
RequestHeader::MaxForwards(Request &req) {

    if (req.getMethod() != "OPTIONS" && req.getMethod() != "TRACE") {
        return CONTINUE;
    }

    int64_t num;
    if (!stoi64(num, value)) {
        return BAD_REQUEST;
    }

    if (num < 0) {
        return BAD_REQUEST;
    } else if (num > 0) {
        num--;
        value = ulltos(num);
    } else {
        req.isProxy(false);
    }

    return CONTINUE;
}

StatusCode
RequestHeader::Origin(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::Pragma(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::ProxyAuthorization(Request &req) {
    (void)req;

    if (!req.isProxy()) {
        Log.debug() << "ProxyAuthorization:: non-proxy server" << Log.endl; 
        return CONTINUE;
    }

    const Auth &auth = req.getLocation()->getAuthRef();
    if (!auth.isSet()) {
        return CONTINUE;
    }

    Log.debug() << "ProxyAuthorization::" << value << Log.endl;
    std::vector<std::string> splitted = split(value, " ");

    if (splitted[0] != "Basic") {
        Log.debug() << "ProxyAuthorization::Only Basic supported for now " << splitted[0] << Log.endl;
        return NOT_IMPLEMENTED;
    }

    std::string decoded = Base64::decode(splitted[1]);
    Log.debug() << "ProxyAuthorization::Decoded Base64:" << decoded << Log.endl;
    if (decoded.empty()) {
        Log.debug() << "ProxyAuthorization::Invalid Base64 string" << Log.endl;
        return PROXY_AUTHENTICATION_REQUIRED;
    }

    req.authorized(auth.isAuthorized(decoded, &req));
    if (req.authorized()) {
        Log.debug() << "ProxyAuthorization::Succeed" << Log.endl;
    } else {
        Log.debug() << "ProxyAuthorization::Failed" << Log.endl;
    }
    
    return CONTINUE;    
}

StatusCode
RequestHeader::Range(Request &req) {
    (void)req;
    
    if (!req.getRangeList().parse(value)) {
        // Not exactly like that, but fine for now
        return RANGE_NOT_SATISFIABLE;
    }
    Log.debug() << "RequestHeader:: Range header detected: " << req.getRangeList()[0].to_string() << Log.endl;

    return CONTINUE;
}

StatusCode
RequestHeader::Referer(Request &req) {

    (void)req;
    // URI ref;    
    // ref.parse(value);
    // if (!ref._path.empty() && ref._path != "/") {
    //     if (endsWith(ref._path, "/")) {
    //         ref._path.erase(ref._path.length() - 1, 1);
    //     }
    // }

    return CONTINUE;
}

StatusCode
RequestHeader::TransferEncoding(Request &req) {
    std::set<std::string> acceptedValues;
    acceptedValues.insert("chunked");

    if (req.has(CONTENT_LENGTH)) {
        Log.debug() << "ContentLength::TransferEncoding: ContentLength header exist" << Log.endl;
        return BAD_REQUEST;
    }

    std::vector<std::string> currentValues = split(this->value, " ,");
    for (std::size_t i = 0; i < currentValues.size(); ++i) {
        if (acceptedValues.find(currentValues[i]) == acceptedValues.end()) {
            Log.error() << "Transfer coding parameter \"" << currentValues[i] << "\" is not supported" << Log.endl;
            return NOT_IMPLEMENTED;
        }
    }
    req.chunked(true);
    req.isChunkSize(true);
    return CONTINUE;
}

StatusCode
RequestHeader::TE(Request &req) {
    if (value == "trailers") {
        req.chunked(true);
    }
    return CONTINUE;
}

StatusCode
RequestHeader::UserAgent(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::Upgrade(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::Via(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::Warning(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::Dnt(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::XRequestedWith(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::XCsrfToken(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::SecFetchDest(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::SecFetchMode(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::SecFetchSite(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::SecFetchUser(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::UpgradeInsecureRequests(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::SecChUa(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::SecGpc(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::SecChUaMobile(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::SecChUaPlatform(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::NotSupported(Request &req) {
    (void)req;
    return CONTINUE;
}

static std::map<uint32_t, RequestHeader::Handler>
initHeadersMap(void) {
    std::map<uint32_t, RequestHeader::Handler> tmp;

    tmp.insert(std::make_pair(A_IM, &RequestHeader::A_IM));
    tmp.insert(std::make_pair(ACCEPT, &RequestHeader::Accept));
    tmp.insert(std::make_pair(ACCEPT_CHARSET, &RequestHeader::AcceptCharset));
    tmp.insert(std::make_pair(ACCEPT_ENCODING, &RequestHeader::AcceptEncoding));
    tmp.insert(std::make_pair(ACCEPT_LANGUAGE, &RequestHeader::AcceptLanguage));
    tmp.insert(std::make_pair(ACCEPT_DATETIME, &RequestHeader::AcceptDateTime));
    tmp.insert(std::make_pair(ACCESS_CONTROL_REQUEST_METHOD, &RequestHeader::AccessControlRequestMethod));
    tmp.insert(std::make_pair(ACCESS_CONTROL_REQUEST_HEADERS, &RequestHeader::AccessControlRequestHeaders));
    tmp.insert(std::make_pair(AUTHORIZATION, &RequestHeader::Authorization));
    tmp.insert(std::make_pair(CACHE_CONTROL, &RequestHeader::CacheControl));
    tmp.insert(std::make_pair(CONNECTION, &RequestHeader::Connection));
    tmp.insert(std::make_pair(CONTENT_LENGTH, &RequestHeader::ContentLength));
    tmp.insert(std::make_pair(CONTENT_TYPE, &RequestHeader::ContentType));
    tmp.insert(std::make_pair(COOKIE, &RequestHeader::Cookie));
    tmp.insert(std::make_pair(DATE, &RequestHeader::Date));
    tmp.insert(std::make_pair(EXPECT, &RequestHeader::Expect));
    tmp.insert(std::make_pair(FORWARDED, &RequestHeader::Forwarded));
    tmp.insert(std::make_pair(FROM, &RequestHeader::From));
    tmp.insert(std::make_pair(HOST, &RequestHeader::Host));
    tmp.insert(std::make_pair(IF_MATCH, &RequestHeader::IfMatch));
    tmp.insert(std::make_pair(IF_MODIFIED_SINCE, &RequestHeader::IfModifiedSince));
    tmp.insert(std::make_pair(IF_NONE_MATCH, &RequestHeader::IfNoneMatch));
    tmp.insert(std::make_pair(IF_RANGE, &RequestHeader::IfRange));
    tmp.insert(std::make_pair(IF_UNMODIFIED_SINCE, &RequestHeader::IfUnmodifiedSince));
    tmp.insert(std::make_pair(MAX_FORWARDS, &RequestHeader::MaxForwards));
    tmp.insert(std::make_pair(ORIGIN, &RequestHeader::Origin));
    tmp.insert(std::make_pair(PRAGMA, &RequestHeader::Pragma));
    tmp.insert(std::make_pair(PROXY_AUTHORIZATION, &RequestHeader::ProxyAuthorization));
    tmp.insert(std::make_pair(RANGE, &RequestHeader::Range));
    tmp.insert(std::make_pair(REFERER, &RequestHeader::Referer));
    tmp.insert(std::make_pair(TRANSFER_ENCODING, &RequestHeader::TransferEncoding));
    tmp.insert(std::make_pair(TE, &RequestHeader::TE));
    tmp.insert(std::make_pair(USER_AGENT, &RequestHeader::UserAgent));
    tmp.insert(std::make_pair(UPGRADE, &RequestHeader::Upgrade));
    tmp.insert(std::make_pair(VIA, &RequestHeader::Via));
    tmp.insert(std::make_pair(WARNING, &RequestHeader::Warning));
    tmp.insert(std::make_pair(DNT, &RequestHeader::Dnt));
    tmp.insert(std::make_pair(X_REQUESTED_WITH, &RequestHeader::XRequestedWith));
    tmp.insert(std::make_pair(X_CSRF_TOKEN, &RequestHeader::XCsrfToken));

    tmp.insert(std::make_pair(SEC_FETCH_DEST, &RequestHeader::SecFetchDest));
    tmp.insert(std::make_pair(SEC_FETCH_MODE, &RequestHeader::SecFetchMode));
    tmp.insert(std::make_pair(SEC_FETCH_SITE, &RequestHeader::SecFetchSite));
    tmp.insert(std::make_pair(SEC_FETCH_USER, &RequestHeader::SecFetchUser));
    tmp.insert(std::make_pair(UPGRADE_INSECURE_REQUESTS, &RequestHeader::UpgradeInsecureRequests));
    tmp.insert(std::make_pair(SEC_CH_UA, &RequestHeader::SecChUa));
    tmp.insert(std::make_pair(SEC_GPC, &RequestHeader::SecGpc));
    tmp.insert(std::make_pair(SEC_CH_UA_MOBILE, &RequestHeader::SecChUaMobile));
    tmp.insert(std::make_pair(SEC_CH_UA_PLATFORM, &RequestHeader::SecChUaPlatform));

    tmp.insert(std::make_pair(PURPOSE, &RequestHeader::NotSupported));

    return tmp;
}

const std::map<uint32_t, RequestHeader::Handler> validReqHeaders = initHeadersMap();

}
