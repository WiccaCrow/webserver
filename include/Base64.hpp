#pragma 

#include <string>

class Base64 {
    static const char *encoders;
    static const char decoders[128];
    
    public:
        static const std::string encode(const std::string &);
        static const std::string decode(const std::string &);
};