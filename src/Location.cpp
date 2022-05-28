#include "Location.hpp"

namespace HTTP
{

    Location::Location() {}
    Location::~Location() {}

    std::string &Location::getPathRef(void)
    {
        return _path;
    }

    bool &Location::getAutoindexRef(void)
    {
        return _autoindex;
    }

    int &Location::getPostMaxBodyRef(void)
    {
        return _post_max_body;
    }

    std::string &Location::getRootRef(void)
    {
        return _root;
    }

    std::string &Location::getDefaultPageRef(void)
    {
        return _defaultPage;
    }

    std::vector<std::string> &Location::getIndexRef(void)
    {
        return _index;
    }

    std::vector<std::string> &Location::getAllowedMethodsRef(void)
    {
        return _allowedMethods;
    }

    std::map<std::string, HTTP::CGI> &Location::getCGIPathsRef(void)
    {
        return _cgiPaths;
    }

    Redirect &Location::getRedirectRef(void)
    {
        return _redirect;
    }
}
