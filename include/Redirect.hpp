#pragma once

#include <string>

class Redirect {

    int         _code;
    std::string _uri;

    bool _set;

public:
    Redirect(void);
    ~Redirect(void);

    bool         isSet(void);
    bool         toggle(void);
    int         &getCodeRef(void);
    std::string &getURIRef(void);
};
