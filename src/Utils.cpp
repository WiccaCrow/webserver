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

// wicca func
std::string getWord_wicca(const std::string& line, char delimiter, size_t& pos) {
    size_t tmp = pos;
    size_t end = pos = line.find(delimiter, pos);
    std::cout << "getWord: test1" << std::endl; // wicca
    if (end == std::string::npos)
        end = line.length();
    std::cout << "getWord: test2" << std::endl;                                          // wicca
    std::cout << "PATH parseLine getWord: " << line.substr(tmp, end - tmp) << std::endl; // wicca
    return line.substr(tmp, end - tmp);
}

std::string getWord(const std::string& line, char delimiter, size_t& pos) {
    size_t tmp = pos;
    size_t end = pos = line.find(delimiter, pos);

    if (end == std::string::npos)
        end = line.length();
    std::cout << "PATH parseLine getWord: " << line.substr(tmp, end - tmp) << std::endl; // wicca
    return line.substr(tmp, end - tmp);
}

void skipSpaces(const std::string& line, size_t& pos) {
    for (; line[pos] == ' '; pos++)
        ;
}

#if __cplusplus < 201103L

std::string to_string(int val) {
    char buf[25];
    snprintf(buf, 25, "%d", val);
    return std::string(buf);
}

#endif
