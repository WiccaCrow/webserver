#include "ValidateHeaders.hpp"

namespace HTTP {

static const uint32 headerHashes[] = {
    360142786,  // a_im
    3005279540, // accept
    379737127,  // accept_charset
    2687938133, // accept_encoding
    480585391,  // accept_language
    1536004560, // accept_datetime
    1698782395, // access_control_request_method
    3862518975, // access_control_request_headers
    2053999599, // authorization
    9184401,    // cache_control
    704082790,  // connection
    314322716,  // content_length
    3266185539, // content_type
    2329983590, // cookie
    2862495610, // date
    482521170,  // expect
    3582250329, // forwarded
    3105530224, // from
    3475444733, // host
    1168849366, // if_match
    1848278858, // if_modified_since
    1529156225, // if_none_match
    2893522586, // if_range
    462614015,  // if_unmodified_since
    2957174193, // max_forwards
    3740358174, // origin
    487986730,  // pragma
    146921266,  // proxy_authorization
    2475121225, // range
    1440495237, // referer
    1470906230, // transfer_encoding
    928136154,  // te
    2191772431, // user_agent
    3076944922, // upgrade
    565660880,  // via
    1078893766, // warning
    478759394,  // dnt
    14045689,   // x_requested_with
    3276872746, // x_csrf_token

    // HTTPS, experimental, etc...
    1781535743, // sec_fetch_dest
    1973190087, // sec_fetch_mode
    2333422472, // sec_fetch_site
    1875242021, // sec_fetch_user
    1981514622, // upgrade_insecure_requests
    3458784846, // sec_ch_ua
    3851717610, // sec_ch_ua_mobile
    464571004,  // sec_ch_ua_platform
};

std::map<uint32, bool> initHashMap(void) {
    std::map<uint32, bool> m;

    const size_t headersCount = sizeof(headerHashes) / sizeof(headerHashes[0]);
    for (size_t i = 0; i < headersCount; ++i) {
        m.insert(std::make_pair(headerHashes[i], true));
    }
    return m;
}

const std::map<uint32, bool> validHeaders = initHashMap();

}; // namespace HTTP