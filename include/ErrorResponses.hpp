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

#define ERROR_RESPONSE_BODY_BASIS \
    "<!DOCTYPE html>\n" \
    "<html lang=en>\n"  \
    "<meta charset=utf-8>\n" \
    "<style>\n" \
    "*{margin:0;padding:0}html{font:15px/22px arial," \
    "sans-serif}html{background:#fff;color:#222;" \
    "padding:15px}body{margin:7% auto 0;max-width:" \
    "390px;min-height:180px;padding:30px 0 15px}p" \
    "{margin:11px 0 22px;overflow:hidden}err_text" \
    "{color:#777;text-decoration:none}" \
    "@media screen and (max-width:772px){body{background:none;" \
    "margin-top:0;max-width:none;padding-right:0}}\n" \
    "</style>\n"

}