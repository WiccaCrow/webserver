#include "Header.hpp"

namespace HTTP {

std::map<std::string, std::string> g_etags;

Header::Header() {}

Header::~Header() {}

Header::Header(uint32_t hash) : hash(hash) {}

Header::Header(uint32_t hash, const std::string &value) : hash(hash), value(value) {}

bool
Header::parse(const std::string &line) {
    
    // Log.debug(line);
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        return false;
    }

    key = line.substr(0, colonPos);
    toLowerCase(key);
    value = line.substr(colonPos + 1);
    trim(value, " \t\r\n");

    hash = crc(key.c_str(), key.length());
    return true;
}

bool operator==(const Header &h1, const Header &h2) {
    return h1.hash == h2.hash;
}

bool operator!=(const Header &h1, const Header &h2) {
    return h1.hash != h2.hash;
}

}

