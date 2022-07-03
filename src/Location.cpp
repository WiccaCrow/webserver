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

int &
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

Location::CGIsMap &
Location::getCGIsRef(void) {
    return _CGIs;
}

Location::ErrorPagesMap &
Location::getErrorPagesRef(void) {
    return _errorPages;
}

Proxy &
Location::getProxyRef(void) {
    return _proxy;
}

const Proxy &
Location::getProxy(void) const {
    return _proxy;
}

Headers<ResponseHeader> &
Location::getHeaders(void) {
    return _headers;
}


}
