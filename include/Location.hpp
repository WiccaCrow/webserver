#pragma once

#include <map>
#include <string>
#include <vector>

#include "CGI.hpp"
#include "Auth.hpp"
#include "Redirect.hpp"

namespace HTTP {

class Location {

public:
    typedef std::vector<std::string>    IndicesVec;
    typedef std::vector<std::string>    MethodsVec;
    typedef std::map<std::string, CGI>  CGIsMap;
    typedef std::vector<std::string>    DomainsVec;
    typedef std::map<int, std::string>  ErrorPagesMap;

private:
    std::string   _path;
    std::string   _root;
    std::string   _alias;
    bool          _autoindex;
    IndicesVec    _index;
    int           _post_max_body;
    MethodsVec    _allowedMethods;
    CGIsMap       _CGIs;
    DomainsVec    _domains;
    Redirect      _redirect;
    Auth          _auth;
    ErrorPagesMap _errorPages;

public:
    Location(void);
    ~Location(void);

    Auth          &getAuthRef(void);
    Redirect      &getRedirectRef(void);
    std::string   &getPathRef(void);
    bool          &getAutoindexRef(void);
    int           &getPostMaxBodyRef(void);
    std::string   &getAliasRef(void);
    std::string   &getRootRef(void);
    IndicesVec    &getIndexRef(void);
    MethodsVec    &getAllowedMethodsRef(void);
    CGIsMap       &getCGIsRef(void);
    DomainsVec    &getDomainsRef(void);
    ErrorPagesMap &getErrorPagesRef(void);
};

}
