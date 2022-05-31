#pragma once

#include <string>
#include <algorithm>

namespace HTTP {

struct URI {

    typedef std::string::iterator iter_t;

    std::string _scheme;
    std::string _host;
    std::string _port;
    std::string _path;
    std::string _query;
    std::string _fragment;

    void parse(std::string uri);
    std::string getAuthority(void) const;

};

}

