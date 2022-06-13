#pragma once

#include <map>
#include <string>

#include "Status.hpp"

namespace HTTP {

class ErrorResponses {
    
    private:
        std::map<int, std::string> _errorResponses;
        const std::string _empty;
    
    public:
        ErrorResponses(void);
        ~ErrorResponses(void);
        const std::string & operator[](HTTP::StatusCode code) const;
        const std::string & operator[](int code) const;

};

extern const ErrorResponses errorResponses;

}