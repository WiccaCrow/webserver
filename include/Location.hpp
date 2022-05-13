#pragma once

#include <string>
#include <vector>
#include <list>

class Location
{
private:
    std::vector<std::pair<std::string, int> > _allowedMethodsAndMaxSizes;
    std::string _path;
    std::string _root;
    bool _autoindex;
    std::list<std::pair<std::string, std::string> > _cgiPaths;

public:
    Location() {}
    ~Location() {}
};