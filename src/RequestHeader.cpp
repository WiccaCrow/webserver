#include "RequestHeader.hpp"
#include "Server.hpp"

namespace HTTP {

StatusCode
RequestHeader::handle(Request &req) {
    std::map<uint32_t, RequestHeader::Handler>::const_iterator it = validReqHeaders.find(hash);

    if (it == validReqHeaders.end()) {
        Log.debug("RequestHeader:: Unknown header: " + key);
        Log.debug("RequestHeader:: Value: " + value);
        Log.debug("RequestHeader:: Hash: " + to_string(hash));
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
    (void)req;
    const Auth &auth = req.getLocation()->getAuthRef();
    if (auth.isSet()) {
        Log.debug("Authorization::" + value);
        std::vector<std::string> splitted = split(value, " ");

        if (splitted[0] != "Basic") {
            Log.debug("Authorization::Only Basic supported for now " + splitted[0]);
            return NOT_IMPLEMENTED;
        }

        uint32_t receivedHash = crc(splitted[1].c_str(), splitted[1].length());
        if (!splitted[1].empty() && receivedHash == req.getStoredHash()) {
            Log.debug("Authorization::Identical hash detected");
            return CONTINUE;
        }

        std::string decoded = Base64::decode(splitted[1]);
        Log.debug("Authorization::Decoded Base64:" + decoded);
        if (decoded.empty()) {
            Log.debug("Authorization::Invalid Base64 string");
            return UNAUTHORIZED;
        }

        req.setAuthFlag(auth.isAuthorized(decoded));
        if (req.isAuthorized()) {
            req.setStoredHash(receivedHash);
            Log.debug("Authorization::Succeed");
        } else {
            Log.debug("Authorization::Failed");
        }
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
    toLowerCase(value);
    if (value != "close" && value.find("close") != std::string::npos) {
        value = "close";
    }
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::ContentLength(Request &req) {
    if (req.isHeaderExist(TRANSFER_ENCODING)) {
        Log.debug("ContentLength::ContentLength: TransferEncoding header exist");
        return BAD_REQUEST;
    }

    long long length;
    if (!stoll(length, value.c_str())) {
        return BAD_REQUEST;
    }
    req.setBodySizeFlag(false);
    req.setBodySize(length);

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
    for (size_t i = 0; i < cookie_pairs.size(); ++i) {
        size_t colonPos = cookie_pairs[i].find(':');
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
    
    URI &host = req.getHostRef();
    host.parse(value);

    if (!isValidHost(host._host)) {
        Log.error("Host: Invalid Host " + host._host);
        return BAD_REQUEST;
    }
    
    if (req.getClient()->getServerPort() != host._port) {
        Log.error("Host: Port mismatch " + host._port_s);
        return BAD_REQUEST;
    }
    
    return CONTINUE;
}

StatusCode
RequestHeader::IfMatch(Request &req) {

    // remove
    if (true)
        return CONTINUE;

    std::vector<std::string> tags = split(value, ", \"");

    if (tags[0] == "*") {
        if (tags.size() == 1) {
            Log.debug("IfMatch:: Any allowed ");
            return CONTINUE;
        } else {
            Log.debug("IfMatch:: Invalid format: " + value);
            // Maybe not bad request
            return BAD_REQUEST;
        }
    }

    std::map<std::string, std::string>::const_iterator itStored;
    itStored = g_etags.find(req.getResolvedPath());
    if (itStored == g_etags.end()) {
        Log.debug("IfMatch:: No etag value found for " + req.getResolvedPath());
        return PRECONDITION_FAILED;
    }

    std::vector<std::string>::iterator itMatched;
    itMatched = std::find(tags.begin(), tags.end(), itStored->second);
    if (itMatched == tags.end()) {
        Log.debug("IfMatch:: None of etag values matched " + *itMatched);
        return PRECONDITION_FAILED;
    }
    return CONTINUE;
}

StatusCode
RequestHeader::IfModifiedSince(Request &req) {
    (void)req;

    if (true)
        return CONTINUE;

    struct tm tm;
    if (!strptime(value.c_str(), "%a, %-e %b %Y %H:%M:%S GMT", &tm)) {
        Log.debug("IfModifiedSince:: Cannot read datetime " + value);
        return BAD_REQUEST;
    }

    // struct stat state;
    if (req.getResolvedPath() != "") {

        // if (stat(req.getResolvedPath().c_str(), &state) < 0) {
        // Log.debug("IfModifiedSince:: ");
        return CONTINUE;
        // }
        // if equal
        // return NOT_MODIFIED;
    }
    return CONTINUE;
}

StatusCode
RequestHeader::IfNoneMatch(Request &req) {
    (void)req;

    // remove
    if (true)
        return CONTINUE;

    std::vector<std::string> tags = split(value, ", \"");

    if (tags[0] == "*") {
        // Another handler
        if (tags.size() == 1) {
            Log.debug("IfNoneMatch:: Any allowed ");
            return CONTINUE;
        } else {
            Log.debug("IfNoneMatch:: Invalid format: " + value);
            // Maybe not bad request
            return BAD_REQUEST;
        }
    }

    std::map<std::string, std::string>::const_iterator itStored;
    itStored = g_etags.find(req.getResolvedPath());
    if (itStored == g_etags.end()) {
        Log.debug("IfNoneMatch:: [OK] No etag value found for " + req.getResolvedPath());
        return CONTINUE;
    }

    std::vector<std::string>::iterator itMatched;
    itMatched = std::find(tags.begin(), tags.end(), itStored->second);
    if (itMatched == tags.end()) {
        Log.debug("IfNoneMatch:: None of etag values matched " + *itMatched);
        return CONTINUE;
    }

    if (req.getMethod() == "GET" || req.getMethod() == "HEAD") {
        return NOT_MODIFIED;
    }
    return PRECONDITION_FAILED;
}

StatusCode
RequestHeader::IfRange(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::IfUnmodifiedSince(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::KeepAlive(Request &req) {
    (void)req;
    return CONTINUE;
}

StatusCode
RequestHeader::MaxForwards(Request &req) {
    (void)req;
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
    return CONTINUE;
}

StatusCode
RequestHeader::Range(Request &req) {
    (void)req;
    Log.debug("RequestHeader:: Range header detected");
    return CONTINUE;
}

StatusCode
RequestHeader::Referer(Request &req) {
    URI ref;
    ref.parse(value);
    (void)req;
    // req.getUriRef()._path = ref._path + req.getUriRef()._path;
    return CONTINUE;
}

StatusCode
RequestHeader::TransferEncoding(Request &req) {
    std::set<std::string> acceptedValues;
    acceptedValues.insert("chunked");

    if (req.isHeaderExist(CONTENT_LENGTH)) {
        Log.debug("ContentLength::TransferEncoding: ContentLength header exist");
        return BAD_REQUEST;
    }

    std::vector<std::string> currentValues = split(this->value, " ,");
    for (size_t i = 0; i < currentValues.size(); ++i) {
        if (acceptedValues.find(currentValues[i]) == acceptedValues.end()) {
            Log.error("Transfer coding parameter \"" + currentValues[i] + "\" is not supported");
            return NOT_IMPLEMENTED;
        }
    }
    req.setBodySizeFlag(true);
    return CONTINUE;
}

StatusCode
RequestHeader::TE(Request &req) {
    if (value == "trailers") {
        req.chuckedRequested(true);
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
