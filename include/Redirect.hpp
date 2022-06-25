#pragma once

#include <string>
#include "Status.hpp"

namespace HTTP {

class Redirect {

    StatusCode  _code;
    std::string _uri;

    bool _set;

public:
    Redirect(void);
    ~Redirect(void);

    bool         set(void);
    void         set(bool);
    StatusCode  &getCodeRef(void);
    std::string &getURIRef(void);
};

}

