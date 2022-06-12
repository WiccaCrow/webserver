#pragma once

#include <algorithm>
#include <stdint.h>

#include "CRC.hpp"
#include "Utils.hpp"
#include "Status.hpp"
#include "Logger.hpp"
#include "Globals.hpp"
#include "HeaderNames.hpp"
#include "HeadersCodes.hpp"

namespace HTTP {

class Header {
    
    public:
    uint32_t    hash;
    std::string key;
    std::string value;

    Header(void);
    virtual ~Header(void);
    Header(uint32_t hash);
    Header(uint32_t hash, const std::string &value);

    virtual bool isValid(void) = 0;

    bool parse(const std::string &line);

    friend bool operator==(const Header &h1, const Header &h2);
    friend bool operator!=(const Header &h1, const Header &h2);
};

} // namespace HTTP
