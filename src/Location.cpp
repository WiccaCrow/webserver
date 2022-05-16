#include "Location.hpp"

Location::Location(){}
Location::~Location(){}

bool & Location::getAutoindexRef(void) {
    return _autoindex;
}

int & Location::getPostMaxBodyRef(void) {
    return _post_max_body;
}

std::string & Location::getRootRef(void) {
    return _root;
}

std::vector<std::string> & Location::getIndexRef(void) {
    return _index;
}

std::vector<std::string> & Location::getAllowedMethodsRef(void) {
    return _allowedMethods;
}

std::map<std::string, std::string> &Location::getCGIPathsRef(void) {
    return _cgiPaths;
}