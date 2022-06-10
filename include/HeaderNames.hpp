#pragma once

#include "HeadersCodes.hpp"
#include <string>
#include <map>

namespace HTTP {

class HeaderNames {
    
    private:
        std::map<uint32_t, std::string> _headerNames;
        const std::string _empty;
    
    public:
        HeaderNames(void);
        ~HeaderNames(void);
        const std::string & operator[](HeaderCode hash) const;
        const std::string & operator[](uint32_t hash) const;

};

extern const HeaderNames headerNames;

}