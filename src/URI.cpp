#include "URI.hpp"

namespace HTTP {

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
        return ;

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
    size_t pos = uri.find("://");
    iter_t schemeEnd = uri.begin();
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

}

