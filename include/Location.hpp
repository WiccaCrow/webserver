#pragma once

#include <map>
#include <string>
#include <vector>

#include "CGI.hpp"
#include "Auth.hpp"
#include "Redirect.hpp"

namespace HTTP {

class Location {

private:
    std::string                _path;
    std::string                _root;
    std::string                _alias;
    bool                       _autoindex;
    std::vector<std::string>   _index;
    int                        _post_max_body;
    std::vector<std::string>   _allowedMethods;
    std::map<std::string, CGI> _CGIs;
    std::vector<std::string>   _domains;
    Redirect                   _redirect;
    Auth                       _auth;
    std::map<int, std::string> _errorPages;

public:
    Location(void);
    ~Location(void);

    Auth                       &getAuthRef(void);
    Redirect                   &getRedirectRef(void);
    std::string                &getPathRef(void);
    bool                       &getAutoindexRef(void);
    int                        &getPostMaxBodyRef(void);
    std::string                &getAliasRef(void);
    std::string                &getRootRef(void);
    std::vector<std::string>   &getIndexRef(void);
    std::vector<std::string>   &getAllowedMethodsRef(void);
    std::map<std::string, CGI> &getCGIsRef(void);
    std::vector<std::string>   &getDomainsRef(void);
    std::map<int, std::string> &getErrorPagesRef(void);
};

}
