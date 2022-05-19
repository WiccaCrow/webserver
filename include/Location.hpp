#pragma once

#include <map>
#include <string>
#include <vector>

class Location {
    private:
    std::string                        _path;
    std::string                        _root;
    bool                               _autoindex;
    std::vector<std::string>           _index;
    int                                _post_max_body;
    std::vector<std::string>           _allowedMethods;
    std::map<std::string, std::string> _cgiPaths;

    public:
    Location(void);
    ~Location(void);
    // Location(Location &other);
    // Location &operator=(Location &other);

    std::string                        &getPathRef(void);
    bool                               &getAutoindexRef(void);
    int                                &getPostMaxBodyRef(void);
    std::string                        &getRootRef(void);
    std::vector<std::string>           &getIndexRef(void);
    std::vector<std::string>           &getAllowedMethodsRef(void);
    std::map<std::string, std::string> &getCGIPathsRef(void);
};