#pragma once

#include <map>
#include <string>
#include <fstream>
#include <unistd.h>

#include "Logger.hpp"

namespace HTTP {

class Auth {

    std::string _realm;
    std::string _file;
    bool        _set;

    std::map<std::string, std::string> _data;

    public:
    Auth(void);
    ~Auth(void);
    bool loadData(void);

    std::string &getFileRef(void);
    std::string &getRealmRef(void);
    bool isSet(void) const;
    void set(bool);
    void setFile(const std::string &);
    void setRealm(const std::string &);

    bool isAuthorized(const std::string &) const;
};

}