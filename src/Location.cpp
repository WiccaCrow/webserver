#include "Location.hpp"

Location::Location() {}
Location::~Location() {}

Location::Location(Location &other) {
    *this = other;
}

Location &Location::operator=(Location &other) {
    if (this != &other) {
        _path = other.getPathRef();
        _root = other.getRootRef();
        _index = other.getIndexRef();
        _cgiPaths = other.getCGIPathsRef();
        _autoindex = other.getAutoindexRef();
        _post_max_body = other.getPostMaxBodyRef();
        _allowedMethods = other.getAllowedMethodsRef();
    }
    return *this;
}

std::string &Location::getPathRef(void) {
    return _path;
}

bool &Location::getAutoindexRef(void) {
    return _autoindex;
}

int &Location::getPostMaxBodyRef(void) {
    return _post_max_body;
}

std::string &Location::getRootRef(void) {
    return _root;
}

std::vector<std::string> &Location::getIndexRef(void) {
    return _index;
}

std::vector<std::string> &Location::getAllowedMethodsRef(void) {
    return _allowedMethods;
}

std::map<std::string, std::string> &Location::getCGIPathsRef(void) {
    return _cgiPaths;
}