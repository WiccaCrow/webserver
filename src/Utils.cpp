#include "Utils.hpp"

static inline std::string& rtrim(std::string& s, const char* t) {
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

static inline std::string& ltrim(std::string& s, const char* t) {
    s.erase(0, s.find_first_not_of(t));
    return s;
}

void trim(std::string& s, const char* t) {
    ltrim(rtrim(s, t), t);
}

void toLowerCase(std::string& s) {
    size_t length = s.length();
    for (size_t i = 0; i < length; ++i) {
        s[i] = tolower(s[i]);
    }
}

std::string to_string(int val) {
    char buf[25];
    snprintf(buf, 25, "%d", val);
    return std::string(buf);
}