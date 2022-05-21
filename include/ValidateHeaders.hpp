#pragma once

#include <stddef.h>

#include <map>

#include "Types.hpp"

namespace HTTP {

enum HeaderCode {
    A_IM=360142786,  // a_im
    ACCEPT=3005279540, // accept
    ACCEPT_CHARSET=379737127,  // accept_charset
    ACCEPT_ENCODING=2687938133, // accept_encoding
    ACCEPT_LANGUAGE=480585391,  // accept_language
    ACCEPT_DATETIME=1536004560, // accept_datetime
    ACCESS_CONTROL_REQUEST_METHOD=1698782395, // access_control_request_method
    ACCESS_CONTROL_REQUEST_HEADERS=3862518975, // access_control_request_headers
    AUTHORIZATION=2053999599, // authorization
    CACHE_CONTROL=9184401,    // cache_control
    CONNECTION=704082790,  // connection
    CONTENT_LENGTH=314322716,  // content_length
    CONTENT_TYPE=3266185539, // content_type
    COOKIE=2329983590, // cookie
    DATE=2862495610, // date
    EXPECT=482521170,  // expect
    FORWARDED=3582250329, // forwarded
    FROM=3105530224, // from
    HOST=3475444733, // host
    IF_MATCH=1168849366, // if_match
    IF_MODIFIED_SINCE=1848278858, // if_modified_since
    IF_NONE_MATCH=1529156225, // if_none_match
    IF_RANGE=2893522586, // if_range
    IF_UNMODIFIED_SINCE=462614015,  // if_unmodified_since
    MAX_FORWARDS=2957174193, // max_forwards
    ORIGIN=3740358174, // origin
    PRAGMA=487986730,  // pragma
    PROXY_AUTHORIZATION=146921266,  // proxy_authorization
    RANGE=2475121225, // range
    REFERER=1440495237, // referer
    TRANSFER_ENCODING=1470906230, // transfer_encoding
    TE=928136154,  // te
    USER_AGENT=2191772431, // user_agent
    UPGRADE=3076944922, // upgrade
    VIA=565660880,  // via
    WARNING=1078893766, // warning
    DNT=478759394,  // dnt
    X_REQUESTED_WITH=14045689,   // x_requested_with
    X_CSRF_TOKEN=3276872746, // x_csrf_token

    // HTTPS, experimental, etc...
    SEC_FETCH_DEST = 1781535743, // sec_fetch_dest
    SEC_FETCH_MODE = 1973190087, // sec_fetch_mode
    SEC_FETCH_SITE = 2333422472, // sec_fetch_site
    SEC_FETCH_USER = 1875242021, // sec_fetch_user
    UPGRADE_INSECURE_REQUESTS = 1981514622, // upgrade_insecure_requests
    SEC_CH_UA = 3458784846, // sec_ch_ua
    SEC_CH_UA_MOBILE = 3851717610, // sec_ch_ua_mobile
    SEC_CH_UA_PLATFORM = 464571004,  // sec_ch_ua_platform
    
};

extern const std::map<uint32, bool> validHeaders;
extern const size_t                 tableSize;

}; // namespace HTTP