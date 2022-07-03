#include "URI.hpp"

namespace HTTP {

URI::URI(void) : _port(0) {}
URI::~URI(void) {}

std::string URI::getAuthority(void) const {
    if (_port != 0) {
        return _host + ":" + _port_s;
    }
    return _host;
}

void URI::clear(void) {
    _scheme = "";
    _host = "";
    _port_s = "";
    _port = 0;
    _path = "";
    _query = "";
    _fragment = "";
}

void URI::parse(std::string uri) {
    if (uri.length() == 0)
        return;

    // fragment
    iter_t fragmentStart = std::find(uri.begin(), uri.end(), '#');
    if (fragmentStart != uri.end()) {
        _fragment = std::string(fragmentStart + 1, uri.end());
    }

    // query
    iter_t queryStart = std::find(uri.begin(), fragmentStart, '?');
    if (queryStart != uri.end()) {
        _query = std::string(queryStart + 1, fragmentStart);
    }

    // scheme
    std::size_t pos = uri.find("://");
    iter_t      schemeEnd = uri.begin();
    if (pos != std::string::npos) {
        std::advance(schemeEnd, pos);
        _scheme = std::string(uri.begin(), schemeEnd);
        std::advance(schemeEnd, 3);
    }

    iter_t pathStart = std::find(schemeEnd, uri.end(), '/');
    if (pathStart != schemeEnd) {
        // host
        iter_t authEnd = pathStart != uri.end() ? pathStart : queryStart;
        iter_t hostEnd = std::find(schemeEnd, authEnd, ':');

        _host = std::string(schemeEnd, hostEnd);

        // port
        if (hostEnd != authEnd) {
            std::advance(hostEnd, 1);
            _port_s = std::string(hostEnd, authEnd);
            char *end = NULL;
            _port = strtoul(_port_s.c_str(), &end, 10);
            if (!end || *end != '\0') {
                _port = -1;
            }
        }
    }

    // path
    if (pathStart != uri.end()) {
        _path = std::string(pathStart, queryStart);
    }
}

std::string URI::URLencode(const std::string &s) {
    static const char shouldBeEncoded[256] = {
        /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
        /* 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 2 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 3 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,

        /* 4 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
        /* 6 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,

        /* 8 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 9 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* A */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* B */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

        /* C */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* D */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* E */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* F */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    const char dec2hex[] = "0123456789ABCDEF";

    std::string result;
    result.reserve(s.length() * 3);
    for (std::size_t i = 0; i < s.length(); i++) {
        if (shouldBeEncoded[static_cast<unsigned char>(s[i])]) {
            result += s[i];
        } else {
            result += '%';
            result += dec2hex[static_cast<unsigned char>(s[i]) >> 4];
            result += dec2hex[static_cast<unsigned char>(s[i]) & 0x0F];
        }
    }

    return result;
}

std::string URI::URLdecode(const std::string &s) {
    static const char hex2dec[256] = {
        /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
        /* 0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 1 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 2 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 3 */ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,

        /* 4 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 5 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 6 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 7 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

        /* 8 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* 9 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* A */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* B */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

        /* C */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* D */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* E */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        /* F */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    if (s.length() < 3) {
        return s;
    }

    std::string result;
    // abcedf 6
    result.reserve(s.length());
    std::size_t i;
    for (i = 0; i < s.length() - 2; i++) {
        if (static_cast<unsigned char>(s[i]) == '%') {
            char d1 = hex2dec[static_cast<unsigned char>(s[i + 1])];
            char d2 = hex2dec[static_cast<unsigned char>(s[i + 2])];
            if (d1 != -1 && d2 != -1) {
                result += (d1 << 4) + d2;
                i += 2;
                continue;
            }
        }
        result += s[i];
    }
    if (i != s.length()) {
        result += s[s.length() - 2];
        result += s[s.length() - 1];
    }
    return result;
}

} // namespace HTTP
