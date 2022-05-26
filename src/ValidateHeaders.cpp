#include "ValidateHeaders.hpp"

namespace HTTP {

static const HeaderCode headerHashes[] = {
    A_IM,
    ACCEPT,
    ACCEPT_CHARSET,
    ACCEPT_ENCODING,
    ACCEPT_LANGUAGE,
    ACCEPT_DATETIME,
    ACCESS_CONTROL_REQUEST_METHOD,
    ACCESS_CONTROL_REQUEST_HEADERS,
    AUTHORIZATION,
    CACHE_CONTROL,
    CONNECTION,
    CONTENT_LENGTH,
    CONTENT_TYPE,
    COOKIE,
    DATE,
    EXPECT,
    FORWARDED,
    FROM,
    HOST,
    IF_MATCH,
    IF_MODIFIED_SINCE,
    IF_NONE_MATCH,
    IF_RANGE,
    IF_UNMODIFIED_SINCE,
    MAX_FORWARDS,
    ORIGIN,
    PRAGMA,
    PROXY_AUTHORIZATION,
    RANGE,
    REFERER,
    TRANSFER_ENCODING,
    TE,
    USER_AGENT,
    UPGRADE,
    VIA,
    WARNING,
    DNT,
    X_REQUESTED_WITH,
    X_CSRF_TOKEN,

    // HTTPS, experimental, etc...
    SEC_FETCH_DEST,
    SEC_FETCH_MODE,
    SEC_FETCH_SITE,
    SEC_FETCH_USER,
    UPGRADE_INSECURE_REQUESTS,
    SEC_CH_UA,
    SEC_GPC,
    SEC_CH_UA_MOBILE,
    SEC_CH_UA_PLATFORM
};

std::map<uint32_t, bool> initHashMap(void) {
    std::map<uint32_t, bool> m;

    const size_t headersCount = sizeof(headerHashes) / sizeof(headerHashes[0]);
    for (size_t i = 0; i < headersCount; ++i) {
        m.insert(std::make_pair(headerHashes[i], true));
    }
    return m;
}

const std::map<uint32_t, bool> validHeaders = initHashMap();

} // namespace HTTP
