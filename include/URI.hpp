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
    static std::string URLencode(const std::string &);
    static std::string URLdecode(const std::string &);
    std::string getAuthority(void) const;
    void clear(void);

};

}

