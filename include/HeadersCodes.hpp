#pragma once


enum HeaderCode {
    A_IM                           = 360142786,
    ACCEPT                         = 3005279540,
    ACCEPT_CHARSET                 = 379737127,
    ACCEPT_ENCODING                = 2687938133,
    ACCEPT_LANGUAGE                = 480585391,
    ACCEPT_DATETIME                = 1536004560,
    ACCESS_CONTROL_REQUEST_METHOD  = 1698782395,
    ACCESS_CONTROL_REQUEST_HEADERS = 3862518975,
    AUTHORIZATION                  = 2053999599,
    CACHE_CONTROL                  = 9184401,
    CONNECTION                     = 704082790,
    CONTENT_LENGTH                 = 314322716,
    CONTENT_TYPE                   = 3266185539,
    COOKIE                         = 2329983590,
    DATE                           = 2862495610,
    EXPECT                         = 482521170,
    FORWARDED                      = 3582250329,
    FROM                           = 3105530224,
    HOST                           = 3475444733,
    IF_MATCH                       = 1168849366,
    IF_MODIFIED_SINCE              = 1848278858,
    IF_NONE_MATCH                  = 1529156225,
    IF_RANGE                       = 2893522586,
    IF_UNMODIFIED_SINCE            = 462614015,
    MAX_FORWARDS                   = 2957174193,
    ORIGIN                         = 3740358174,
    PRAGMA                         = 487986730,
    PROXY_AUTHORIZATION            = 146921266,
    RANGE                          = 2475121225,
    REFERER                        = 1440495237,
    TRANSFER_ENCODING              = 1470906230,
    TE                             = 928136154,
    USER_AGENT                     = 2191772431,
    UPGRADE                        = 3076944922,
    VIA                            = 565660880,
    WARNING                        = 1078893766,
    DNT                            = 478759394,
    X_REQUESTED_WITH               = 14045689,
    X_CSRF_TOKEN                   = 3276872746,

    // HTTPS
    SEC_FETCH_DEST            = 1781535743,
    SEC_FETCH_MODE            = 1973190087,
    SEC_FETCH_SITE            = 2333422472,
    SEC_FETCH_USER            = 1875242021,
    UPGRADE_INSECURE_REQUESTS = 1981514622,
    SEC_CH_UA                 = 3458784846,
    SEC_GPC                   = 570648046,
    SEC_CH_UA_MOBILE          = 3851717610,
    SEC_CH_UA_PLATFORM        = 464571004,

    // Chrome's non standard header
    PURPOSE = 3095901163,

    // for response
    ACCEPT_PATCH = 108822466,
    ACCEPT_RANGES = 2930364553,
    AGE = 2704281778,
    ALLOW = 2118729960,
    ALT_SVC = 1794662009,
    CONTENT_DISPOSITION = 3607300749,
    CONTENT_ENCODING = 3836410099,
    CONTENT_LANGUAGE = 1480159241,
    CONTENT_LOCATION = 3531376759,
    CONTENT_RANGE = 1051632209,
    DELTA_BASE = 2836946796,
    ETAG = 3514087100,
    EXPIRES = 2593941644,
    IM = 3336953844,
    LAST_MODIFIED = 4183802465,
    LINK = 917281265,
    LOCATION = 1587448267,
    PROXY_AUTHENTICATE = 2775274866,
    PUBLIC_KEY_PINS = 3246089802,
    RETRY_AFTER = 882486162,
    SERVER = 1517147638,
    SET_COOKIE = 1431457525,
    STRICT_TRANSPORT_SECURITY = 2020361446,
    TRAILER = 3331447886,
    TK = 3505002205,
    VARY = 315970471,
    WWW_AUTHENTICATE = 1561078874,
    ACCESS_CONTROL_ALLOW_ORIGIN = 3687209729,
    ACCESS_CONTROL_ALLOW_CREDENTIALS = 3062691792,
    ACCESS_CONTROL_EXPOSE_HEADERS = 3597970766,
    ACCESS_CONTROL_MAX_AGE = 3530484745,
    ACCESS_CONTROL_ALLOW_METHODS = 3689926033,
    ACCESS_CONTROL_ALLOW_HEADERS = 2047767933,
    CONTENT_SECURITY_POLICY = 1685543244,
    REFRESH = 1619108091,
    X_POWERED_BY = 4092125758,
    X_REQUEST_ID = 3793395544,
    X_UA_COMPATIBLE = 354887779,
    X_XSS_PROTECTION = 2696822975
};
