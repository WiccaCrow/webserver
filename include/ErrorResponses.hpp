#pragma once

#include <map>
#include <string>

#include "Status.hpp"

namespace HTTP {

class ErrorResponses {
    
    private:
        std::map<int, std::string> _errorResponses;
    
    public:
        ErrorResponses(void);
        ~ErrorResponses(void);
        bool has(StatusCode code) const;
        bool has(int code) const;
        const std::string & operator[](HTTP::StatusCode code) const;
        const std::string & operator[](int code) const;

};

extern const ErrorResponses errorResponses;

}