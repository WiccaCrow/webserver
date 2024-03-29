#pragma once

#include <map>
#include <string>
#include <vector>

#include "CGI.hpp"
#include "Auth.hpp"
#include "Proxy.hpp"
#include "Redirect.hpp"

namespace HTTP {

class Location {

public:
    typedef std::vector<std::string>    MethodsVec;
    typedef std::vector<std::string>    IndicesVec;
    typedef std::map<std::string, CGI>  CGIsMap;
    typedef std::map<int, std::string>  ErrorPagesMap;

private:
    std::string   _path;
    std::string   _root;
    std::string   _alias;
    bool          _autoindex;
    IndicesVec    _index;
    uint64_t      _post_max_body;
    MethodsVec    _allowedMethods;
    MethodsVec    _cgiMethods;
    CGIsMap       _CGIs;
    Redirect      _redirect;
    Auth          _auth;
    ErrorPagesMap _errorPages;
    URI           _proxy_pass;
    Headers<ResponseHeader> _headers;

public:
    Location(void);
    ~Location(void);

    Auth          &getAuthRef(void);
    Redirect      &getRedirectRef(void);
    std::string   &getPathRef(void);
    bool          &getAutoindexRef(void);
    uint64_t      &getPostMaxBodyRef(void);
    std::string   &getAliasRef(void);
    std::string   &getRootRef(void);
    IndicesVec    &getIndexRef(void);
    MethodsVec    &getAllowedMethodsRef(void);
    MethodsVec    &getCGIMethodsRef(void);
    CGIsMap       &getCGIsRef(void);
    ErrorPagesMap &getErrorPagesRef(void);
    URI           &getProxyPassRef(void);
    const URI     &getProxyPass(void) const;

    Headers<ResponseHeader>   &getHeaders(void);
};

}
