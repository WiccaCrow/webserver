#include "ValidateHeaders.hpp"

namespace HTTP {

static const uint32 hashHeaders[] = {
    360142786,  // a-im
    3005279540, // accept
    379737127,  // accept-charset
    2687938133, // accept-encoding
    480585391,  // accept-language
    1536004560, // accept-datetime
    1698782395, // access-control-request-method
    3862518975, // access-control-request-headers
    2053999599, // authorization
    9184401,    // cache-control
    704082790,  // connection
    314322716,  // content-length
    3266185539, // content-type
    2329983590, // cookie
    2862495610, // date
    482521170,  // expect
    3582250329, // forwarded
    3105530224, // from
    3475444733, // host
    1168849366, // if-match
    1848278858, // if-modified-since
    1529156225, // if-none-match
    2893522586, // if-range
    462614015,  // if-unmodified-since
    2957174193, // max-forwards
    3740358174, // origin
    487986730,  // pragma
    146921266,  // proxy-authorization
    2475121225, // range
    1440495237, // referer
    928136154,  // te
    2191772431, // user-agent
    3076944922, // upgrade
    565660880,  // via
    1078893766, // warning
    478759394,  // dnt
    14045689,   // x-requested-with
    3276872746  // x-csrf-token
};

std::vector<bool> initHashTable(void);

const size_t            headersCount = sizeof(hashHeaders) / sizeof(hashHeaders[0]);
const std::vector<bool> validHeaders = initHashTable();
const size_t            tableSize = 193;

std::vector<bool> initHashTable(void) {
    std::vector<bool> vec(tableSize, false);

    for (size_t i = 0; i < headersCount; ++i) {
        size_t index = hashHeaders[i] % tableSize;
        vec[index] = true;
    }
    return vec;
}

}; // namespace HTTP