#pragma once

#include <string>
#include <algorithm>

namespace HTTP {

struct URI {

    typedef std::string::iterator iter_t;

    std::string _scheme;
    std::string _host;
    std::string _port_s;
    std::string _path;
    std::string _query;
    std::string _fragment;
    size_t _port;

    void parse(std::string uri);
    std::string encode(const std::string & sSrc);
    std::string decode(const std::string & sSrc);
    std::string getAuthority(void) const;
    void clear(void);

};

}

