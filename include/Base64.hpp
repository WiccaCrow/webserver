#pragma once

#include <string>
#include <string.h>

class Base64 {
    static const char *encoders;
    static const char decoders[128];
    
    public:
        static const std::string encode(const std::string &);
        static const std::string decode(const std::string &);
        static bool isValid(const std::string &s);
};