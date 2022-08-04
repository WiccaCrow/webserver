#include "Location.hpp"

namespace HTTP {

Location::Location() { }
Location::~Location() { }

std::string &
Location::getPathRef(void) {
    return _path;
}

Auth &
Location::getAuthRef(void) {
    return _auth;
}

bool &
Location::getAutoindexRef(void) {
    return _autoindex;
}

uint64_t &
Location::getPostMaxBodyRef(void) {
    return _post_max_body;
}

std::string &
Location::getAliasRef(void) {
    return _alias;
}

std::string &
Location::getRootRef(void) {
    return _root;
}

Redirect &
Location::getRedirectRef(void) {
    return _redirect;
}

Location::IndicesVec &
Location::getIndexRef(void) {
    return _index;
}

Location::MethodsVec &
Location::getAllowedMethodsRef(void) {
    return _allowedMethods;
}

Location::MethodsVec &
Location::getCGIMethodsRef(void) {
    return _cgiMethods;
}

Location::CGIsMap &
Location::getCGIsRef(void) {
    return _CGIs;
}

Location::ErrorPagesMap &
Location::getErrorPagesRef(void) {
    return _errorPages;
}

URI &
Location::getProxyPassRef(void) {
    return _proxy_pass;
}

const URI &
Location::getProxyPass(void) const {
    return _proxy_pass;
}

Headers<ResponseHeader> &
Location::getHeaders(void) {
    return _headers;
}


}
