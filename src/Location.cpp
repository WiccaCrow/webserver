#include "Location.hpp"

namespace HTTP {

Location::Location() { }
Location::~Location() { }

std::string &
Location::getPathRef(void) {
    return _path;
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

std::vector<std::string> &
Location::getIndexRef(void) {
    return _index;
}

std::vector<std::string> &
Location::getAllowedMethodsRef(void) {
    return _allowedMethods;
}

std::map<std::string, HTTP::CGI> &
Location::getCGIsRef(void) {
    return _CGIs;
}

Redirect &
Location::getRedirectRef(void) {
    return _redirect;
}
}
