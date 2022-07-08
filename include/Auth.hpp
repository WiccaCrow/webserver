#pragma once

#include <map>
#include <string>
#include <fstream>
#include <unistd.h>

#include "Logger.hpp"
#include "Utils.hpp"

namespace HTTP {

class Request;

class Auth {

    std::string _realm;
    std::string _file;
    std::string _type;
    bool        _set;

    std::map<std::string, std::string> _data;

    public:
    Auth(void);
    ~Auth(void);
    bool loadData(void);

    std::string &getFileRef(void);
    std::string &getRealmRef(void);
    std::string getType(void) const;
    bool isSet(void) const;
    void set(bool);
    void setFile(const std::string &);
    void setRealm(const std::string &);

    bool isAuthorized(const std::string, Request *req) const;
};

}